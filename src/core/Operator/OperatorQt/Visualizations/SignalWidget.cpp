//////////////////////////////////////////////////////////////////////////////
// $Id: SignalWidget.cpp 7195 2023-02-07 18:31:03Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A widget to display signals from a BCI2000 file.
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
//////////////////////////////////////////////////////////////////////////////
#include "SignalWidget.h"
#include <QPaintEvent>
#include <QPalette>
#include <QPainter>

static const int cFocusFrameWidth = 4;

SignalWidget::SignalWidget(QWidget *inParent)
    : QWidget(inParent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setFocusPolicy(Qt::StrongFocus);
    setEnabled(true);
    mDisplay.SetSelectionColor(RGBColor::FromQRgb(palette().highlight().color().rgba()));
    UpdateDrawContext();
}

SignalDisplay &SignalWidget::Display()
{
    return mDisplay;
}

const SignalDisplay &SignalWidget::Display() const
{
    return mDisplay;
}

void SignalWidget::paintEvent(QPaintEvent *inpEvent)
{
    mDisplay.Paint(&(inpEvent->region()));
    if (hasFocus()) {
        QPainter painter(this);
        QPen pen = painter.pen();
        pen.setColor(palette().color(QPalette::Highlight));
        pen.setWidth(cFocusFrameWidth);
        painter.setPen(pen);
        QRect rect = this->rect().adjusted(cFocusFrameWidth / 2, cFocusFrameWidth / 2, -cFocusFrameWidth / 2, -cFocusFrameWidth / 2);
        painter.drawRect(rect);
    }
}

void SignalWidget::resizeEvent(QResizeEvent *inpEvent)
{
    UpdateDrawContext();
    QWidget::resizeEvent(inpEvent);
}

void SignalWidget::UpdateDrawContext()
{
    GUI::DrawContext dc = {{0, 0, this->width(), this->height()}, 0};
    dc.device = this;
    mDisplay.SetContext(dc);
}
