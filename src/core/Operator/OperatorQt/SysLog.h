//////////////////////////////////////////////////////////////////////
// $Id: SysLog.h 9599 2026-10-15 16:48:06Z mellinger $
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: The Operator module's log window.
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
#ifndef SYSLOG_H
#define SYSLOG_H

#include <QDialog>
#include <QMutex>
#include <QString>
#include <QTextBrowser>
#include <chrono>
#include <queue>
#include <mutex>

class MainWindow;

class SysLog : public QDialog
{
    Q_OBJECT

  public:
    enum LogEntryMode
    {
        logEntryNormal = 0,
        logEntryWarning,
        logEntryError,
        numLogEntryModes
    };
    SysLog(MainWindow *);
    virtual ~SysLog();

    static SysLog* Instance();

    bool HasUndisplayedErrors()
    {
        return !mErrorsDisplayedSoFar;
    }

    void PushEntry(std::chrono::system_clock::time_point, const QString& text, int = logEntryNormal, bool verbatim = false);
    void PopEntries();

  public slots:
    void AddEntry(std::chrono::system_clock::time_point, const QString &text, int = logEntryNormal, bool verbatim = false);
    void Move(int x, int y) { QWidget::move(x, y); }
    void Resize(int w, int h) { QWidget::resize(w, h); }

  protected:
    void moveEvent(QMoveEvent *) override;
    void showEvent(QShowEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void closeEvent(QCloseEvent *) override;

  private:
    bool MatchURL(const QString &, int &, int &);
    void Show(bool force);

    MainWindow *mpParent;
    QTextBrowser *mpLog;
    QTextCharFormat mDefaultFormat;

    struct Entry {
        std::chrono::system_clock::time_point when;
        QString text;
        int mode;
        bool verbatim;
    };
    std::queue<Entry> mEntryQueue;
    std::mutex mQueueMutex;

    bool mEmpty, mWasVisible, mAttachToParent, mErrorsDisplayedSoFar;
    enum
    {
        normal,
        closeDenied,
        closed
    } mState;
    QString mLastLine;
    QStringMatcher mURLMatcher;
};

#endif // SYSLOG_H
