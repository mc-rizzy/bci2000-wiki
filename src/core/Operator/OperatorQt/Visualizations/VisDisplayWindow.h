////////////////////////////////////////////////////////////////////////////////
// $Id: VisDisplayWindow.h 9472 2026-07-18 21:00:52Z mellinger $
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
#ifndef VIS_DISPLAY_WINDOW_H
#define VIS_DISPLAY_WINDOW_H

#include "VisDisplayBase.h"

class VisualizationDock;
class VisDisplayContainer;

class VisDisplayWindow : public VisDisplayBase
{
    Q_OBJECT

  public:
    VisDisplayWindow(const std::string &visID, bool createLayout = true);
    virtual ~VisDisplayWindow();

    std::string WindowID() const
    {
        return mVisID.WindowID();
    }

  public slots:
    void SetVisible(bool) override;

  public:
    static void SetDockWindow(VisualizationDock*);
    static void SetWindowMenu(QMenu *);

  protected:
    void OnSetConfig(ConfigSettings &) override;

  private:
    void EnsureDockWidget();
    void RemoveDockWidget();

    void OnMove(QMoveEvent*);
    void OnResize(QResizeEvent*);
    void OnClose(QCloseEvent*);
    void OnTopLevelChanged(bool);

    static void buildWindowMenu(QMenu *);

    struct Private;
    Private* p;
};

#endif // VIS_DISPLAY_WINDOW_H