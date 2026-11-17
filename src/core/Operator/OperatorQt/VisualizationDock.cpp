//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: The Operator module's visualization dock window.
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
///////////////////////////////////////////////////////////////////////
#include "VisualizationDock.h"

#include "MainWindow.h"
#include "SysLog.h"
#include "NotesWindow.h"
#include "WatchesWindow.h"
#include "Settings.h"
#include "Preferences.h"
#include "Debugging.h"
#include "VisDisplayBase.h"

#include <QGroupBox>
#include <QBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QSizePolicy>
#include <QTimer>

DockWidget::DockWidget()
{
}

void DockWidget::keyPressEvent(QKeyEvent* iopEvent)
{
    if (iopEvent->key() == Qt::Key_Tab && iopEvent->modifiers() == Qt::CTRL) {
        emit onCtrlTab();
        iopEvent->accept();
        return;
    }
    if (iopEvent->key() == Qt::Key_Backtab && iopEvent->modifiers() == (Qt::CTRL | Qt::SHIFT)) {
        emit onCtrlShiftTab();
        iopEvent->accept();
        return;
    }
    QDockWidget::keyPressEvent(iopEvent);
}

void DockWidget::moveEvent(QMoveEvent* iopEvent)
{
    emit onMove(iopEvent);
    QDockWidget::moveEvent(iopEvent);
}

void DockWidget::resizeEvent(QResizeEvent* iopEvent)
{
    emit onResize(iopEvent);
    QDockWidget::resizeEvent(iopEvent);
}

void DockWidget::closeEvent(QCloseEvent* iopEvent)
{
    emit onClose(iopEvent);
    QDockWidget::closeEvent(iopEvent);
}

// VisualizationDock
static const int kInitialSize = 460;
static const int kInitialPos = 10;
#if __APPLE__
static const int kSingleWindowModeLeftMargin = 2;
#else
static const int kSingleWindowModeLeftMargin = 9;
#endif
static const int kMainControlsMinHeight = 100;
static const int kMainControlsMaxHeight = 125;


VisualizationDock* VisualizationDock::spInstance;

VisualizationDock::VisualizationDock(MainWindow* pMainWindow)
{
    if (!Preferences::Instance()->mSingleWindowMode) {
        setWindowTitle(Preferences::Instance()->mInstancePrefix + pMainWindow->windowTitle() + " Watches and Visualizations");
        setAttribute(Qt::WA_QuitOnClose, false);
        Qt::WindowFlags flags = windowFlags();
        flags = flags & (~Qt::WindowCloseButtonHint);
        setWindowFlags(flags);
    }
    else {
        setWindowTitle(Preferences::Instance()->mInstancePrefix + pMainWindow->windowTitle());
        setAttribute(Qt::WA_QuitOnClose, true);
        Qt::WindowFlags flags = windowFlags();
        flags = flags | Qt::WindowCloseButtonHint;
        setWindowFlags(flags);
    }

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setDockNestingEnabled(true);
    setCentralWidget(WatchesWindow::Instance());
    connect(WatchesWindow::Instance(), &WatchesWindow::CtrlTabPressed, this, &VisualizationDock::OnCtrlTab);
    connect(WatchesWindow::Instance(), &WatchesWindow::CtrlShiftTabPressed, this, &VisualizationDock::OnCtrlShiftTab);
    connect(this, &QMainWindow::tabifiedDockWidgetActivated, this, &VisualizationDock::OnTabifiedDockWidgetActivated);
    resize(kInitialSize, kInitialSize);
    move(kInitialPos, kInitialPos);
    Restore();

    if (!Preferences::Instance()->mSingleWindowMode) {
        mpMenuEntry = pMainWindow->GetWindowMenu()->addAction("Visualization Dock");
        mpMenuEntry->setShortcut(QKeySequence("Ctrl+Alt+V"));
        mpMenuEntry->setCheckable(true);
        connect(mpMenuEntry, &QAction::triggered, this, &QWidget::setVisible);
        mpMenuEntry->setChecked(this->isVisible());

        this->setWindowState(Qt::WindowNoState);
    }

    if (Preferences::Instance()->mSingleWindowMode) {
    
        // rearrange main controls widget
        auto pGroupBox = new QGroupBox;
        auto pLayout = new QHBoxLayout;
        pLayout->setContentsMargins(0, 0, 0, 0);
        pMainWindow->layout()->setContentsMargins(0, 11, 0, 11);
        pMainWindow->setMinimumHeight(kMainControlsMinHeight);
        pMainWindow->setMaximumHeight(kMainControlsMaxHeight);
        pLayout->addWidget(pMainWindow);
        pGroupBox->setLayout(pLayout);
        pGroupBox->setMaximumHeight(kMainControlsMaxHeight);
        auto pMainControlsWidget = new QWidget;
        pLayout = new QHBoxLayout;
        pLayout->setContentsMargins(kSingleWindowModeLeftMargin, 0, 0, 0);
        pLayout->addWidget(pGroupBox);
        pMainControlsWidget->setLayout(pLayout);
        pMainControlsWidget->setMaximumHeight(kMainControlsMaxHeight);

        auto pStatusBar = pMainWindow->findChild<QStatusBar*>();
        if (pStatusBar) {
            pStatusBar->hide();
        }
        auto pMenuBar = pMainWindow->findChild<QMenuBar*>();
        if (pMenuBar) {
            pMenuBar->setParent(this, pMenuBar->windowFlags());
            this->layout()->setMenuBar(pMenuBar);
        }

        auto pDockWidget = new DockWidget;
        pDockWidget->setObjectName("MainControls");
        pDockWidget->setWindowTitle(" Main Controls");
        pDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        pDockWidget->setWidget(pMainControlsWidget);
        this->addDockWidget(Qt::LeftDockWidgetArea, pDockWidget);
        mSingleWindowDockWidgets.push_back(pDockWidget);
        mSingleWindowDockWidgetSizes.push_back(1);

        // add SysLog windows as a dock widget
        auto pSysLogWindow = SysLog::Instance();
        pDockWidget = new DockWidget;
        pSysLogWindow->layout()->setContentsMargins(kSingleWindowModeLeftMargin, 0, 0, 0);
        pDockWidget->setObjectName("SysLog");
        pDockWidget->setWindowTitle(" BCI2000 Log");
        pDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        pDockWidget->setWidget(pSysLogWindow);
        this->addDockWidget(Qt::LeftDockWidgetArea, pDockWidget);
        mSingleWindowDockWidgets.push_back(pDockWidget);
        mSingleWindowDockWidgetSizes.push_back(50);

        // add Notes window as a dock widget
        auto pNotesWindow = NotesWindow::Instance();
        pDockWidget = new DockWidget;
        pNotesWindow->layout()->setContentsMargins(kSingleWindowModeLeftMargin, 0, 0, 0);
        pDockWidget->setObjectName("Notes");
        pDockWidget->setWindowTitle(" Notes");
        pDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        pDockWidget->setWidget(pNotesWindow);
        pNotesWindow->hide();
        this->addDockWidget(Qt::LeftDockWidgetArea, pDockWidget);
        mSingleWindowDockWidgets.push_back(pDockWidget);
        mSingleWindowDockWidgetSizes.push_back(50);

        this->setWindowState(Qt::WindowMaximized);
    }
    // This is a workaround: dock windows cannot be resized
    // before construction is finished, so we use a single shot
    // timer that triggers as soon as the main event loop takes over.
    QTimer::singleShot(0, this, &VisualizationDock::AfterConstruction);
}

VisualizationDock::~VisualizationDock()
{
    Save();
    centralWidget()->setParent(nullptr);
    setCentralWidget(nullptr);
    WatchesWindow::Deinitialize();
    for (auto& entry : mVisualizationDockWidgets) {
        entry.second->setParent(nullptr);
        QWidget* pWidget = entry.second->widget();
        if (pWidget)
            pWidget->setParent(nullptr);
        delete entry.second;
        entry.second = nullptr;
    }

    for (auto& pDockWidget : mSingleWindowDockWidgets) {
        auto pWidget = pDockWidget->widget();
        pDockWidget->setWidget(nullptr);
        if (pWidget)
            pWidget->setParent(nullptr);
    }
}

DockWidget* VisualizationDock::GetDockWidget(const std::string& visID)
{
    DockWidget* pDockWidget = mVisualizationDockWidgets[visID];
    if (!pDockWidget) {
        pDockWidget = CreateDockWidget(visID);
        Qt::DockWidgetArea area = Qt::TopDockWidgetArea;
        if (visID == "RNDT")
            area = Qt::BottomDockWidgetArea;
        pDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
        this->addDockWidget(area, pDockWidget);
        if (visID != "RNDT") {
            auto pFirstTab = GetFirstTopAreaWidget(visID);
            if (pFirstTab)
                this->tabifyDockWidget(pFirstTab, pDockWidget);
        }
    }
    show();
    return pDockWidget;
}

void VisualizationDock::OnDockWidgetAssociated(const std::string& visID, DockWidget* pDockWidget)
{
    pDockWidget->show();
    // Raise our dock widget area's last activated item.
    auto area = dockWidgetArea(pDockWidget);
    auto i = mLastActivatedTabifiedDockWidgets.find(area);
    if (i != mLastActivatedTabifiedDockWidgets.end()) {
        const std::string& lastActivatedVisID = i->second;
        auto j = mVisualizationDockWidgets.find(lastActivatedVisID);
        if (j != mVisualizationDockWidgets.end()) {
            auto area2 = dockWidgetArea(j->second);
            if (area2 == area)
                j->second->raise();
        }
    }
}

VisualizationDock* VisualizationDock::Instance()
{
    return spInstance;
}

void VisualizationDock::Initialize(MainWindow* inpParent)
{
    Assert(spInstance == nullptr);
    spInstance = new VisualizationDock(inpParent);
}

void VisualizationDock::Deinitialize()
{
    delete spInstance;
    spInstance = nullptr;
}

void VisualizationDock::Move(int x, int y)
{
    move(x, y);
}

void VisualizationDock::Resize(int h, int w)
{
    resize(h, w);
}

void VisualizationDock::keyPressEvent(QKeyEvent* iopEvent)
{
    if (iopEvent->key() == Qt::Key_Tab && iopEvent->modifiers() == Qt::CTRL) {
        OnCtrlTab();
        iopEvent->accept();
        return;
    }
    if (iopEvent->key() == Qt::Key_Backtab && iopEvent->modifiers() == (Qt::CTRL | Qt::SHIFT)) {
        OnCtrlShiftTab();
        iopEvent->accept();
        return;
    }
    QMainWindow::keyPressEvent(iopEvent);
}

void VisualizationDock::showEvent(QShowEvent*)
{
    if (mpMenuEntry)
        mpMenuEntry->setChecked(isVisible());
}

void VisualizationDock::closeEvent(QCloseEvent* ev)
{
    if (mpMenuEntry)
        mpMenuEntry->setChecked(false);

    if (Preferences::Instance()->mSingleWindowMode) {
        if (!MainWindow::Instance()->close())
            ev->ignore();
    }
}

DockWidget* VisualizationDock::GetFirstTopAreaWidget(const std::string& visID)
{
    for (const auto& entry : mVisualizationDockWidgets) {
        if (entry.first == visID)
            continue;
        auto area = dockWidgetArea(entry.second);
        if (area != Qt::TopToolBarArea)
            continue;
        return entry.second;
    }
    return nullptr;
}

DockWidget* VisualizationDock::CreateDockWidget(const std::string& key)
{
    auto pDockWidget = new DockWidget;
    pDockWidget->setObjectName(QString::fromUtf8(key.c_str()));

    if (!Preferences::Instance()->mSingleWindowMode) {
        pDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    }
    else {
        pDockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
        pDockWidget->setFeatures(pDockWidget->features() & ~QDockWidget::DockWidgetFloatable);
    }

    pDockWidget->setMinimumHeight(200); // workaround to avoid timing window to be compressed to zero height

    mVisualizationDockWidgets[key] = pDockWidget;

    connect(pDockWidget, &DockWidget::onCtrlTab, this, &VisualizationDock::OnCtrlTab);
    connect(pDockWidget, &DockWidget::onCtrlShiftTab, this, &VisualizationDock::OnCtrlShiftTab);

    return pDockWidget;
}

DockWidget* VisualizationDock::GetNextDockWidget(DockWidget* inpCurrent, int direction)
{
    struct WidgetSorter { bool operator()(const std::string & lhs, const std::string & rhs) const {
        if(lhs == "SRCD" && rhs != "SRCD") return true; // sort the source signal pane to the beginning
        if(lhs != "SRCD" && rhs == "SRCD") return false;
        if(lhs == "RNDT" && rhs != "RNDT") return false; // sort the timing pane to the end
        if(lhs != "RNDT" && rhs == "RNDT") return true;
        return lhs < rhs;
    } };
    std::map<std::string, DockWidget*, WidgetSorter> reorderedDockWidgets;
    for (const auto& entry : mVisualizationDockWidgets) reorderedDockWidgets[entry.first] = mVisualizationDockWidgets[entry.first];

    DockWidget* pPrevious = nullptr;
    if (direction > 0)
    {
        for (const auto& entry : reorderedDockWidgets) {
            if (pPrevious == inpCurrent)
                return entry.second;
            pPrevious = entry.second;
        }
    }
    if (direction < 0)
    {
        for (auto entry = reorderedDockWidgets.rbegin(); entry != reorderedDockWidgets.rend(); ++entry)
        {
            if (pPrevious == inpCurrent)
                return entry->second;
            pPrevious = entry->second;
        }
    }
    return nullptr;
}

void VisualizationDock::Save() const
{
    Settings settings(Preferences::Instance()->mInstancePrefix);
    settings.beginGroup("Config");

    settings.beginGroup("LastActivatedTabifiedDockWidgets");
    for (const auto& entry : mLastActivatedTabifiedDockWidgets)
        settings.setValue(QString::number(entry.first), entry.second.c_str());
    settings.endGroup();

    settings.beginGroup("VisualizationDock");
    for (const auto& entry : mVisualizationDockWidgets)
        settings.setValue(entry.first.c_str(), dockWidgetArea(entry.second));
    settings.endGroup();

    if (!Preferences::Instance()->mSingleWindowMode) {
        settings.setValue("DockState", saveState());
        settings.setValue("Geometry", saveGeometry());
    }
    else {
        settings.setValue("SwmDockState", saveState());
        settings.setValue("SwmGeometry", saveGeometry());
    }
}

void VisualizationDock::Restore()
{
    Settings settings(Preferences::Instance()->mInstancePrefix);
    settings.beginGroup("Config");

    mLastActivatedTabifiedDockWidgets.clear();
    settings.beginGroup("LastActivatedTabifiedDockWidgets");
    auto childKeys = settings.childKeys();
    for (auto& key : childKeys)
        mLastActivatedTabifiedDockWidgets[Qt::DockWidgetArea(key.toInt())] = settings.value(key).toString().toUtf8().constData();
    settings.endGroup();

    // Saving and restoring empty dock widgets is necessary because dock windows need to exist
    // in order for QMainWindow::restoreState() to work properly.
    settings.beginGroup("VisualizationDock");
    childKeys = settings.childKeys();
    if (!childKeys.contains("RNDT"))
        childKeys.push_back("RNDT");
    for (auto& key : childKeys) {
        auto pDockWidget = CreateDockWidget(key.toUtf8().constData());
        Qt::DockWidgetArea area = Qt::DockWidgetArea(settings.value(key).toInt());
        if (area == 0) {
            area = Qt::TopDockWidgetArea;
            if (key == "RNDT") {
                area = Qt::BottomDockWidgetArea;
            }
        }
        addDockWidget(area, pDockWidget);
    }
    settings.endGroup();

    if (!Preferences::Instance()->mSingleWindowMode) {
        QByteArray geom = settings.value("Geometry").toByteArray();
        restoreGeometry(geom);

        QByteArray state = settings.value("DockState").toByteArray();
        restoreState(state);
    }
    else {
        QByteArray geom = settings.value("SwmGeometry").toByteArray();
        restoreGeometry(geom);

        QByteArray state = settings.value("SwmDockState").toByteArray();
        restoreState(state);
    }

    for (const auto& entry : mVisualizationDockWidgets)
        entry.second->hide();
}

void VisualizationDock::OnCtrlTab()
{
    SwitchTabs(+1);
}
void VisualizationDock::OnCtrlShiftTab()
{
    SwitchTabs(-1);
}
void VisualizationDock::OnTabifiedDockWidgetActivated(QDockWidget* pWidget)
{
    auto pActiveWidget = dynamic_cast<DockWidget*>(pWidget);
    if (pActiveWidget) {
        mpCurrentFocusWidget = pActiveWidget;
        auto pVisDisplay = mpCurrentFocusWidget->widget()->findChild<VisDisplayBase*>();
        if (pVisDisplay) {
            pVisDisplay->setFocus();
            mLastActivatedTabifiedDockWidgets[dockWidgetArea(pWidget)] = pVisDisplay->ID().WindowID();
        }
    }
}
void VisualizationDock::SwitchTabs(int direction)
{
    bool actionOccurred = false;
    while (!actionOccurred) {
        mpCurrentFocusWidget = GetNextDockWidget(mpCurrentFocusWidget, direction);

        if (!mpCurrentFocusWidget) {
            centralWidget()->setFocus();
            centralWidget()->activateWindow();
            actionOccurred = true;
        }
        else if (mpCurrentFocusWidget->widget() && mpCurrentFocusWidget->isVisible()) {
            if (!this->tabifiedDockWidgets(mpCurrentFocusWidget).empty()) {
                mpCurrentFocusWidget->raise();
                actionOccurred = true;
            }
            if (mpCurrentFocusWidget->isFloating()) {
                mpCurrentFocusWidget->activateWindow();
            }
            else {
                this->activateWindow();
            }
            QWidget* pVisDisplay = mpCurrentFocusWidget->widget()->findChild<VisDisplayBase*>();
            if (pVisDisplay) {
                pVisDisplay->setFocus();
                actionOccurred = true;
            }
        }
    }
}

void VisualizationDock::AfterConstruction()
{
    if (!mSingleWindowDockWidgets.empty()) {
        this->resizeDocks(mSingleWindowDockWidgets, mSingleWindowDockWidgetSizes, Qt::Vertical);
    }
}
