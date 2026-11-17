//////////////////////////////////////////////////////////////////////
// $Id: WatchesWindow.h 9216 2026-01-01 14:20:40Z mellinger $
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A window displaying user-editable expressions.
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
#ifndef WATCHES_WINDOW_H
#define WATCHES_WINDOW_H

#include <QWidget>
#include <QTableWidget>
#include "ThreadUtils.h"

class WatchesTable : public QTableWidget
{
    Q_OBJECT
  public:
    WatchesTable(QWidget* parent);
  signals:
    void CtrlTabPressed();
    void CtrlShiftTabPressed();
  protected:
    void keyPressEvent(class QKeyEvent*) override;
};

class WatchesWindow : public QWidget
{
    Q_OBJECT
  public:
    static double constexpr nan = std::numeric_limits<double>::quiet_NaN();
    static void Initialize(class MainWindow *);
    static void Deinitialize();
    static WatchesWindow *Instance();

    void SetBlockDurationSeconds(double);

  public slots:
    void AddWatch(const QString& ,int decimate = -1, double rangeMin = nan, double rangeMax = nan);
    void MarkMostRecentSamples();
    void MarkNextSamples();
    void Move(int x, int y) { QWidget::move(x, y); }
    void Resize(int w, int h) { QWidget::resize(w, h); }

  protected:
    void keyPressEvent(class QKeyEvent *) override;
    void timerEvent(class QTimerEvent *) override;
    void paintEvent(class QPaintEvent *) override;

  signals:
    void CtrlTabPressed();
    void CtrlShiftTabPressed();

  private:
    WatchesWindow(class MainWindow *);
    ~WatchesWindow();
    void Update();
    struct Watch;

  private slots:
    void OnItemChanged(class QTableWidgetItem *);
    void OnValuesReceived(void *, const QStringList&);
    void OnHeaderDoubleClick(int);
    void AddEmptyLine();
    void RemoveLastLine();
    void SetHistoryLengthSeconds(double);
    void SaveWatches();
    void RestoreWatches();
    void ClearWatches();

  private:
    WatchesTable *mpTable;
    double mHistoryLengthSeconds, mBlockDurationSeconds, mUpdateInterval, mLastUpdate, mWatchesNow;
    ThreadUtils::TimeSlice mTimeSlice;
};

#endif // WATCHES_WINDOW_H
