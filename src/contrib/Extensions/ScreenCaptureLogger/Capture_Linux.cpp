/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A function that captures a screen area into memory as a
//   BGRA image.
// 
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
/////////////////////////////////////////////////////////////////////////////
#include "Capture.h"

#include "EnvVariable.h"
#include "StringUtils.h"

extern "C" {
#include <libswscale/swscale.h>
}
#include <sys/ipc.h>
#include <sys/shm.h>
#include <xcb/xcb.h>
#include <xcb/shm.h>

#include <stdexcept>

struct Capture::Private
{
    Capture* mpSelf = nullptr;

    int mLeft = 0, mTop = 0, mWidth = 0, mHeight = 0;
    float mDecimation = 1;
    SwsContext* mpScalingContext = nullptr;
    xcb_connection_t* mpConnection = nullptr;

    int mShmid = -1;
    void* mpShmaddr = nullptr;
    xcb_shm_seg_t mShmseg = {0};

    void EnsureScalingContext();
    void ClearScalingContext();

    void EnsureSharedMemory();
    void ReleaseSharedMemory();
};

Capture::Capture()
: p(new Private)
{
    p->mpSelf = this;
    std::string sessionType = EnvVariable("XDG_SESSION_TYPE").Get();
    sessionType = StringUtils::ToLower(sessionType);
    if (!sessionType.empty()) {
        if (sessionType != "x11" && sessionType != "xorg") {
            throw std::runtime_error("Not an X11 session, use ScreenCaptureLogger_Wayland");
        }
    }
    p->mpConnection = ::xcb_connect(nullptr, nullptr);
    if (::xcb_connection_has_error(p->mpConnection)) {
        throw std::runtime_error("Cannot connect to X server");
    }
}

Capture::~Capture()
{
    if (p->mpConnection)
        ::xcb_disconnect(p->mpConnection);
    p->ClearScalingContext();
    p->ReleaseSharedMemory();
    delete p;
}

void Capture::Private::EnsureScalingContext()
{
    if (mpScalingContext)
        return;

    int outWidth = mpSelf->GetOutputWidth();
    int outHeight = mpSelf->GetOutputHeight();
    auto pCtx = ::sws_getContext(mWidth, mHeight, AV_PIX_FMT_BGRA, outWidth, outHeight, AV_PIX_FMT_BGRA, SWS_POINT, nullptr, nullptr, nullptr);
    if (!pCtx) {
        throw std::runtime_error("Could not allocate scaling context");
    }
    mpScalingContext = pCtx;
}

void Capture::Private::ClearScalingContext()
{
    if (mpScalingContext)
        ::sws_freeContext(mpScalingContext);
    mpScalingContext = nullptr;
}

void Capture::Private::EnsureSharedMemory()
{
    if (mShmid >= 0)
        return;

    mShmid = ::shmget(IPC_PRIVATE, mWidth * mHeight * 4, IPC_CREAT | 0600);
    mpShmaddr = ::shmat(mShmid, NULL, 0);
    mShmseg = ::xcb_generate_id(mpConnection);
    ::xcb_shm_attach(mpConnection, mShmseg, mShmid, 0); // 0 = read-write
}

void Capture::Private::ReleaseSharedMemory()
{
    if (mShmid >=0) {
        ::xcb_shm_detach(mpConnection, mShmseg);
        ::shmdt(mpShmaddr);
        mpShmaddr = nullptr;
        ::shmctl(mShmid, IPC_RMID, nullptr);
        mShmid = -1;
    }
}

Capture& Capture::SetRect(int x, int y, int width, int height)
{
    p->ClearScalingContext();
    p->ReleaseSharedMemory();

    p->mLeft = x;
    p->mTop = y;
    p->mWidth = width;
    p->mHeight = height;
    return *this;
}

Capture& Capture::SetDecimation(float d)
{
    p->ClearScalingContext();
    p->ReleaseSharedMemory();

    p->mDecimation = ::std::max(d, 1.f);
    return *this;
}

Capture& Capture::StartupCaptureStream()
{
    return *this;
}

int Capture::GetOutputWidth() const
{
    return p->mWidth / p->mDecimation;
}

int Capture::GetOutputHeight() const
{
    return p->mHeight / p->mDecimation;
}

int Capture::GetImageBytes() const
{
    return GetOutputWidth() * GetOutputHeight() * 4;
}

void Capture::DoCapture(void* pMemory)
{
    p->EnsureSharedMemory();
    xcb_screen_t* screen = ::xcb_setup_roots_iterator(::xcb_get_setup(p->mpConnection)).data;
    xcb_shm_get_image_cookie_t cookie = ::xcb_shm_get_image(
        p->mpConnection,
        screen->root,
        p->mLeft, p->mTop,
        p->mWidth, p->mHeight,
        ~0,
        XCB_IMAGE_FORMAT_Z_PIXMAP,
        p->mShmseg, 0
    );

    xcb_shm_get_image_reply_t* reply = ::xcb_shm_get_image_reply(p->mpConnection, cookie, nullptr);
    if (reply) {
        if (p->mDecimation == 1) {
            ::memcpy(pMemory, p->mpShmaddr, GetImageBytes());
        }
        else {
            p->EnsureScalingContext();
            int srcStride = p->mWidth * 4,
                destStride = GetOutputWidth() * 4;
            uint8_t* pSrc = static_cast<uint8_t*>(p->mpShmaddr);
            uint8_t* pDest = static_cast<uint8_t*>(pMemory);
            ::sws_scale(p->mpScalingContext, &pSrc, &srcStride, 0, p->mHeight, &pDest, &destStride);
        }
        free(reply);
    } else {
        throw std::runtime_error("Failed to get image reply");
    }

}
