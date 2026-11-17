////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A helper class that allows access to a QToolButton's button.
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
#include <QToolButton>
#include <QWidgetAction>

class ToolButtonAction : public QWidgetAction
{
public:
    ToolButtonAction(const QString& text, QMenu* pMenu, QToolButton** ppButton = nullptr)
    : QWidgetAction(nullptr), mText(text), mpMenu(pMenu), mppButton(ppButton)
    {
    }
    ~ToolButtonAction()
    {
        if (mppButton)
            *mppButton = nullptr;
    }
    QWidget* createWidget(QWidget* parent) override
    {
        QToolButton* pButton = new QToolButton(parent);
        pButton->setText(mText);
        pButton->setToolTip(mText);
        if (mpMenu) {
            pButton->setMenu(mpMenu);
            pButton->setPopupMode(QToolButton::InstantPopup);
        }
        if (mppButton)
            *mppButton = pButton;
        return pButton;
    }
private:
    QString mText;
    QMenu* mpMenu = nullptr;
    QToolButton** mppButton = nullptr;
};
