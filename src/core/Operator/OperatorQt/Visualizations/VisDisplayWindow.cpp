////////////////////////////////////////////////////////////////////////////////
// $Id: VisDisplayWindow.cpp 9472 2026-07-18 21:00:52Z mellinger $
// Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de
// Description: Root window class for visualization displays.
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
#include "VisDisplayWindow.h"
#include "VisDisplayContainer.h"
#include "Preferences.h"
#include "Debugging.h"
#include "Settings.h"
#include "VisualizationDock.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QScreen>
#include <QStackedLayout>
#include <QDockWidget>
#include <QMainWindow>

#include <cctype>
#include <map>

static VisualizationDock *spParentWindow = nullptr;

typedef std::pair<VisID, VisDisplayBase *> Visual;

static size_t stripTrailingNumber(const std::string &s)
{
    size_t pos = s.length();
    while (pos > 0 && std::isdigit(s[pos - 1]))
        --pos;
    while (pos > 0 && std::isspace(s[pos - 1]))
        --pos;
    if (pos == 0 || pos == s.length())
        pos = std::string::npos;
    return pos;
}

static void addWindowAction(QMenu *pMenu, const Visual &visual)
{
    VisDisplayBase *pWindow = visual.second;
    QString title = pWindow->windowTitle();
    title += (" [" + visual.first.WindowID() + "]").c_str();
    auto action = pMenu->addAction(title);
    action->setCheckable(true);
    action->setChecked(pWindow->isVisible());
    action->connect(action, &QAction::triggered, pWindow, &VisDisplayBase::SetVisible);
}

struct VisDisplayWindow::Private
{
    QStackedLayout* mpLayout = nullptr;
    std::atomic<DockWidget*> mpDockWidget = nullptr;
    bool mUserIsMoving = true;
};

void VisDisplayWindow::SetWindowMenu(QMenu *pMenu)
{
    pMenu->connect(pMenu, &QMenu::aboutToShow, [pMenu]() { buildWindowMenu(pMenu); });
}

void VisDisplayWindow::SetDockWindow(VisualizationDock* p)
{
    spParentWindow = p;
}

void VisDisplayWindow::buildWindowMenu(QMenu* pMenu)
{
    std::map<std::string, std::vector<Visual>> windows;
    for (const auto& v : Visuals())
    {
        if (dynamic_cast<VisDisplayWindow*>(v.second))
        {
            size_t pos = stripTrailingNumber(v.first);
            windows[v.first.Substr(0, pos)].push_back(v);
        }
    }

    pMenu->clear();
    for (const auto& w : windows)
    {
        if (w.second.size() == 1)
        {
            addWindowAction(pMenu, w.second.front());
        }
        else
        {
            std::string title = w.second.front().second->windowTitle().toLocal8Bit().constData();
            size_t pos = stripTrailingNumber(title);
            if (pos == std::string::npos)
                title.clear();
            else
                title = title.substr(0, pos) + ' ';
            title += "[" + w.first + "]";
            auto pSubmenu = pMenu->addMenu(QString::fromLocal8Bit(title.c_str()));
            for (const auto& visual : w.second)
                addWindowAction(pSubmenu, visual);
        }
    }
    if (pMenu->isEmpty())
        pMenu->addAction("(no visualizations)")->setEnabled(false);
}

// Member functions
VisDisplayWindow::VisDisplayWindow(const std::string &inVisID, bool createLayout)
: VisDisplayBase(inVisID), p(new Private)
{
    if (createLayout)
    {
        p->mpLayout = new QStackedLayout(this);
        p->mpLayout->setStackingMode(QStackedLayout::StackAll);
    }

    // All VisDisplayWindows have a black background for backward compatibility
    // Note: This palette will propagate to child QWidgets.
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Window, QBrush(Qt::black));
    pal.setBrush(QPalette::WindowText, QBrush(Qt::white));
    this->setPalette(pal);

    EnsureDockWidget();
}

VisDisplayWindow::~VisDisplayWindow()
{
    RemoveDockWidget();
    delete p;
}

void VisDisplayWindow::EnsureDockWidget()
{
    if (!p->mpDockWidget) {
        p->mpDockWidget = spParentWindow->GetDockWidget(mVisID);
        connect(p->mpDockWidget, &DockWidget::onMove, this, &VisDisplayWindow::OnMove);
        connect(p->mpDockWidget, &DockWidget::onResize, this, &VisDisplayWindow::OnResize);
        connect(p->mpDockWidget, &DockWidget::onClose, this, &VisDisplayWindow::OnClose);
        connect(p->mpDockWidget, &DockWidget::topLevelChanged, this, &VisDisplayWindow::OnTopLevelChanged);
        p->mpDockWidget.load()->setWidget(this);
        spParentWindow->OnDockWidgetAssociated(mVisID, p->mpDockWidget);
    }
}

void VisDisplayWindow::RemoveDockWidget()
{
    if (p->mpDockWidget) {
        p->mpDockWidget.load()->setWidget(nullptr);
        disconnect(p->mpDockWidget, &DockWidget::onMove, this, &VisDisplayWindow::OnMove);
        disconnect(p->mpDockWidget, &DockWidget::onResize, this, &VisDisplayWindow::OnResize);
        disconnect(p->mpDockWidget, &DockWidget::onClose, this, &VisDisplayWindow::OnClose);
        disconnect(p->mpDockWidget, &DockWidget::topLevelChanged, this, &VisDisplayWindow::OnTopLevelChanged);
        p->mpDockWidget.load()->hide();
        this->setParent(nullptr);
        p->mpDockWidget = nullptr;
    }
}

void VisDisplayWindow::SetVisible(bool b)
{
    if (p->mpDockWidget) {
        p->mpDockWidget.load()->setVisible(b);
    }
    else {
        setVisible(b);
    }
}

void VisDisplayWindow::OnSetConfig(ConfigSettings &inConfig)
{
    p->mUserIsMoving = false;

    std::string title = inConfig[CfgID::WindowTitle];
    if (!title.empty())
        setWindowTitle(Preferences::Instance()->mInstancePrefix + title.c_str());
    else
        setWindowTitle(Preferences::Instance()->mInstancePrefix + mVisID.c_str());

    if (p->mpDockWidget) {
        p->mpDockWidget.load()->setWindowTitle(windowTitle());

        bool docked = true;
        if (Visconfigs()[mVisID].Get(CfgID::Docked, docked))
            p->mpDockWidget.load()->setFloating(!docked);
    }

    const int minSize = 100;
    int formTop = 0, formLeft = 0, formHeight = 0, formWidth = 0;
    bool ok = inConfig.Get(CfgID::Top, formTop) && inConfig.Get(CfgID::Left, formLeft)
              && inConfig.Get(CfgID::Height, formHeight) && inConfig.Get(CfgID::Width, formWidth);
    if (ok) {
        QRect formRect(formLeft, formTop, formWidth, formHeight);
        formRect.adjust(minSize, minSize, -minSize, -minSize);
        auto screens = QGuiApplication::screens();
        bool isVisible = false;
        for (auto screen : std::as_const(screens))
            isVisible |= formRect.intersects(screen->availableGeometry());

        if (!isVisible && !screens.empty())
        {
            formTop = 10;
            formLeft = 10;
        }
        if (p->mpDockWidget && p->mpDockWidget.load()->isFloating()) {
            p->mpDockWidget.load()->move(formLeft, formTop);
            p->mpDockWidget.load()->resize(formWidth, formHeight);
        }
    }

    bool windowFrame;
    if (inConfig.Get(CfgID::WindowFrame, windowFrame))
    {
        QWidget* pWidget = this;
        if (p->mpDockWidget)
            pWidget = p->mpDockWidget;
        Qt::WindowFlags flags = pWidget->windowFlags() & ~Qt::FramelessWindowHint;
        if (!windowFrame)
            flags |= Qt::FramelessWindowHint;
        pWidget->setWindowFlags(flags);
    }

    std::string containerVisID;
    if (inConfig.Get(CfgID::PlacementVis, containerVisID))
    {
        int row = 1, col = 1, height = 1, width = 1;
        inConfig.Get(CfgID::PlacementRow, row);
        inConfig.Get(CfgID::PlacementCol, col);
        inConfig.Get(CfgID::PlacementHeight, height);
        inConfig.Get(CfgID::PlacementWidth, width);
        auto pContainer = dynamic_cast<VisDisplayContainer*>(Visuals()[containerVisID]);
        if (!pContainer)
        {
            VisDisplayBase::Create<VisDisplayContainer>(containerVisID);
            pContainer = dynamic_cast<VisDisplayContainer*>(Visuals()[containerVisID]);
        }
        Assert(pContainer);
        auto pSelf = dynamic_cast<VisDisplayContainer*>(this);
        if (pSelf && pSelf->ContainsSubwindow(pContainer))
            throw std_runtime_error << "Detected recursive placement";
        RemoveDockWidget();
        pContainer->PlaceSubwindow(this, row - 1, col - 1, height, width);
    }
    else
    {
        EnsureDockWidget();
    }
    VisDisplayBase::OnSetConfig(inConfig);
    p->mUserIsMoving = true;
}

void VisDisplayWindow::OnMove(QMoveEvent* iopEvent)
{
    if (p->mUserIsMoving)
    {
        VisDisplayBase::Visconfigs()[mVisID].Put(CfgID::Left, p->mpDockWidget.load()->pos().x(), UserDefined);
        VisDisplayBase::Visconfigs()[mVisID].Put(CfgID::Top, p->mpDockWidget.load()->pos().y(), UserDefined);
    }
}

void VisDisplayWindow::OnResize(QResizeEvent* iopEvent)
{
    if (p->mUserIsMoving)
    {
        VisDisplayBase::Visconfigs()[mVisID].Put(CfgID::Width, p->mpDockWidget.load()->size().width(), UserDefined);
        VisDisplayBase::Visconfigs()[mVisID].Put(CfgID::Height, p->mpDockWidget.load()->size().height(), UserDefined);
    }
}

void VisDisplayWindow::OnClose(QCloseEvent* iopEvent)
{
    // If the VisDisplay is a dependant of another window
    // (i.e. has been created via direct user interaction),
    // destroy it rather than hiding it.
    if (mVisID.IsDependant()) {
        auto children = findChildren<VisDisplayBase*>();
        for (auto pChild : children) {
            VisDisplay::Destroy(pChild->ID());
        }
        VisDisplay::Destroy(mVisID);
    }
}

void VisDisplayWindow::OnTopLevelChanged(bool isFloating)
{
    VisDisplayBase::Visconfigs()[mVisID].Put(CfgID::Docked, !isFloating, UserDefined);
}
