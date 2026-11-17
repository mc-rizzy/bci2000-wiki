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
#include <QMainWindow>
#include <QDockWidget>
#include <QMoveEvent>
#include <QResizeEvent>

#include <map>

class MainWindow;

class DockWidget : public QDockWidget
{
    Q_OBJECT

public:
    DockWidget();

signals:
    void onCtrlTab();
    void onCtrlShiftTab();
    void onMove(QMoveEvent*);
    void onResize(QResizeEvent*);
    void onClose(QCloseEvent*);

protected:
    void keyPressEvent(QKeyEvent*) override;
    void moveEvent(QMoveEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void closeEvent(QCloseEvent*) override;
};


class VisualizationDock : public QMainWindow
{
    Q_OBJECT

public:
    VisualizationDock(MainWindow* parent);
    ~VisualizationDock();

    DockWidget* GetDockWidget(const std::string& visID);
    void OnDockWidgetAssociated(const std::string& visID, DockWidget*);

    static VisualizationDock* Instance();
    static void Initialize(MainWindow*);
    static void Deinitialize();

public slots:
    void Move(int x, int y);
    void Resize(int h, int w);

protected:
    void keyPressEvent(QKeyEvent*) override;
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;

private:
    DockWidget* GetFirstTopAreaWidget(const std::string&);
    DockWidget* CreateDockWidget(const std::string&);
    DockWidget* GetNextDockWidget(DockWidget*, int direction=+1);

    void Save() const;
    void Restore();

    void OnCtrlTab();
    void OnCtrlShiftTab();
    void OnTabifiedDockWidgetActivated(QDockWidget*);
    void SwitchTabs(int direction);

    void AfterConstruction();

    std::map<std::string, DockWidget*> mVisualizationDockWidgets;
    std::map<Qt::DockWidgetArea, std::string> mLastActivatedTabifiedDockWidgets;
    DockWidget* mpCurrentFocusWidget = nullptr;
    QAction* mpMenuEntry = nullptr;

    QList<QDockWidget*> mSingleWindowDockWidgets;
    QList<int> mSingleWindowDockWidgetSizes;

    static VisualizationDock* spInstance;
};
