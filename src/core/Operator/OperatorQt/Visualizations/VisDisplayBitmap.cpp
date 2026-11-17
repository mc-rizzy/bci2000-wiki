////////////////////////////////////////////////////////////////////////////////
// $Id: VisDisplayBitmap.cpp 9494 2026-07-22 12:43:22Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class for bitmap type visualization windows.
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
////////////////////////////////////////////////////////////////////////////////
#include "VisDisplayBitmap.h"
#include "SynchronizedObject.h"
#include <QPaintEvent>
#include <QPainter>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

struct VisDisplayBitmap::Private
{
    bool mWaitForReferenceFrame;
    BitmapImage mFrame;
};

VisDisplayBitmap::VisDisplayBitmap(const std::string &inSourceID) : VisDisplayLayer(inSourceID), p(nullptr)
{
    setVisible(false);
    p = new Private;
    SyncMemory<AfterWrite>();
    this->show();
}

VisDisplayBitmap::~VisDisplayBitmap()
{
    setVisible(false);
    SyncMemory<AfterWrite>();
    delete p;
}

void VisDisplayBitmap::OnSetConfig(ConfigSettings &inConfig)
{
    VisDisplayBase::OnSetConfig(inConfig);
}

void VisDisplayBitmap::OnBitmap(const BitmapImage &inImage)
{
    if (inImage.Empty())
    {
        p->mWaitForReferenceFrame = true;
        return;
    }

    if (p->mWaitForReferenceFrame)
    {
        if ((p->mFrame.Width() != inImage.Width()) || (p->mFrame.Height() != inImage.Height()))
        {
            // Adapt the window's aspect ratio without changing its width.
            if (this->hasFocus() && inImage.Width() > 0)
            {
                QWidget *pParent = this->parentWidget();
                pParent->resize(pParent->width(), (inImage.Height() * pParent->width()) / inImage.Width());
                Visconfigs()[mVisID.WindowID()].Put(CfgID::Height, pParent->height(), UserDefined);
            }
        }
        p->mFrame = inImage;
        p->mWaitForReferenceFrame = false;
    }
    else
    {
        p->mFrame += inImage;
    }
    this->update();
}

void VisDisplayBitmap::paintEvent(QPaintEvent *iopEvent)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::TextAntialiasing, false);

    // Always draw the full image with proper aspect ratio.
    QRect rect = this->rect();
    int width = p->mFrame.Width(), height = p->mFrame.Height();
    float ratio = 1.0f * width / height;
    if (rect.width() / ratio > rect.height()) {
        int correctedWidth = rect.height() * ratio;
        rect.setLeft(rect.left() + (rect.width() - correctedWidth) / 2);
        rect.setWidth(correctedWidth);
    }
    else if (rect.height() * ratio > rect.width()) {
        int correctedHeight = rect.width() / ratio;
        rect.setTop(rect.top() + (rect.height() - correctedHeight) / 2);
        rect.setHeight(correctedHeight);
    }

    int targetLeft = rect.left(), targetTop = rect.top();
    int targetWidth = rect.width(), targetHeight = rect.height();
    if (p && !p->mFrame.Empty())
    {
        for (int x = 0; x < p->mFrame.Width(); ++x)
        {
            for (int y = 0; y < p->mFrame.Height(); ++y)
            {
                RGBColor rgb = p->mFrame(x, y);
                if (rgb != RGBColor::NullColor)
                {
                    QRect pixelRect;
                    pixelRect.setLeft(targetLeft + (x * targetWidth) / p->mFrame.Width());
                    pixelRect.setTop(targetTop + (y * targetHeight) / p->mFrame.Height());
                    pixelRect.setRight(targetLeft + ((x + 1) * targetWidth) / p->mFrame.Width());
                    pixelRect.setBottom(targetTop + ((y + 1) * targetHeight) / p->mFrame.Height());
                    painter.fillRect(pixelRect, QColor(rgb.R(), rgb.G(), rgb.B()));
                }
            }
        }
#ifdef _WIN32
        ::Sleep(0);
#endif // _WIN32
    }
    VisDisplayBase::paintEvent(iopEvent);
    iopEvent->accept();
}

void VisDisplayBitmap::mousePressEvent(QMouseEvent *iopEvent)
{
    this->parentWidget()->activateWindow();
    VisDisplayBase::mousePressEvent(iopEvent);
}
