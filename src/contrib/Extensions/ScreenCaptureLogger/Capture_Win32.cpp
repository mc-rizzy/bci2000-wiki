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

#include "SysError.h"
#include "Exception.h"

#include <Windows.h>

struct Capture::Private
{
    int x = 0, y = 0, width = 0, height = 0;
    int decimation = 1;
};

Capture::Capture()
: p(new Private)
{
}

Capture::~Capture()
{
    delete p;
}

Capture& Capture::SetRect(int x, int y, int width, int height)
{
    p->x = x;
    p->y = y;
    p->width = width;
    p->height = height;
    return *this;
}

Capture& Capture::SetDecimation(float d)
{
    p->decimation = std::max(d, 1.f);
    return *this;
}

Capture& Capture::StartupCaptureStream()
{
    return *this;
}

int Capture::GetOutputWidth() const
{
    return p->width / p->decimation;
}

int Capture::GetOutputHeight() const
{
    return p->height / p->decimation;
}

int Capture::GetImageBytes() const
{
    return GetOutputWidth() * GetOutputHeight() * 4;
}

void Capture::DoCapture(void* pMemory)
{
    int outWidth = GetOutputWidth(), outHeight = GetOutputHeight();

    bool success = false;

    // Get desktop DC
    HDC hDesktopDC = ::GetDC(NULL);
    if (hDesktopDC) {
        // Create memory DC compatible with desktop DC
        HDC hMemoryDC = ::CreateCompatibleDC(hDesktopDC);
        if (hMemoryDC) {
            // Create bitmap compatible with desktop DC
            HBITMAP hBitmap = ::CreateCompatibleBitmap(hDesktopDC, outWidth, outHeight);
            if (hBitmap) {
                // Select bitmap into memory DC
                HBITMAP hOldBitmap = reinterpret_cast<HBITMAP>(::SelectObject(hMemoryDC, hBitmap));
                // StretchBlt to capture the region
                ::SetStretchBltMode(hMemoryDC, STRETCH_DELETESCANS);
                if (::StretchBlt(hMemoryDC, 0, 0, outWidth, outHeight, hDesktopDC, p->x, p->y, p->width, p->height, SRCCOPY)) {

                    BITMAPINFOHEADER bmi = { 0 };
                    bmi.biSize = sizeof(BITMAPINFOHEADER);
                    bmi.biWidth = outWidth;
                    bmi.biHeight = -outHeight; // flip image vertically
                    bmi.biPlanes = 1;
                    bmi.biBitCount = 32;
                    bmi.biCompression = BI_RGB;
                    bmi.biSizeImage = 0;

                    // Get bitmap info and pixel data
                    BITMAPINFO bmiInfo = { 0 };
                    bmiInfo.bmiHeader = bmi;

                    if (::GetDIBits(hMemoryDC, hBitmap, 0, outHeight, pMemory, &bmiInfo, DIB_RGB_COLORS)) {
                        success = true;
                    }
                }
                ::SelectObject(hMemoryDC, hOldBitmap);
                ::DeleteObject(hBitmap);
            }
            ::DeleteDC(hMemoryDC);
        }
        ::ReleaseDC(NULL, hDesktopDC);
    }
    if (!success) {
        throw std_runtime_error << SysError().Message();
    }
}

