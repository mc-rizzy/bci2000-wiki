////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A proxy class that allows to call certain non-slot QWidget
//   member functions across threads. Useful e.g. to invalidate only a region
//   of a window from a secondary thread.
//   Currently, only the update(const QRegion&) member function is callable
//   through QtWidgetProxy but any desired function may be added in the 
//   same manner.
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
#ifndef QT_WIDGET_PROXY_H
#define QT_WIDGET_PROXY_H

#include <QWidget>
#include "Debugging.h"

class QtWidgetProxy : public QObject
{
    Q_OBJECT
public:
    QtWidgetProxy(QWidget* pClient = nullptr) : mpClient(pClient)
    {
        if (pClient) { Assert(pClient->thread() == this->thread()); }
    }

    QtWidgetProxy(const QtWidgetProxy&) = delete;
    QtWidgetProxy& operator=(const QtWidgetProxy&) = delete;

public slots:
    void UpdateRegion(QRegion rgn)
    {
        if (mpClient)
            mpClient->update(rgn);
    }

private:
    QWidget* mpClient = nullptr;
};

#endif // QT_WIDGET_PROXY_H
