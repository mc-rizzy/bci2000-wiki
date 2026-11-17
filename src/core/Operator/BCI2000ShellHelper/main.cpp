//////////////////////////////////////////////////////////////////////////////
// $Id: main.cpp 9570 2026-09-22 16:45:17Z mellinger $
// Author: mellinger@neurotechcenter.org
// Description: BCI2000ShellHelper: runs BCI2000 batch files on macOS
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
#include "QtMain.h"
#include "FileUtils.h"
#include "SysError.h"
#include <unistd.h>
#include <QApplication>
#include <QTimer>
#include <QMessageBox>
#include <QFileInfo>

struct FileOpenHandler : QObject
{
    FileOpenHandler()
    {
        connect(&mTimer, &QTimer::timeout, this, &FileOpenHandler::onTimeout);
        mTimer.start(1000);
    }

    void onFileOpen(const QString& file)
    {
        mTimer.stop();
        QFileInfo info(file);
        FileUtils::ChangeDirectory(info.absolutePath().toLocal8Bit().constData());
        std::string executable = info.absoluteFilePath().toLocal8Bit().constData();

        char* argv[] = { const_cast<char*>(executable.c_str()), nullptr };
        if (::execv(executable.c_str(), argv) < 0) {
            int err = errno;
            QMessageBox::critical(
                nullptr, "Error running batch file",
                QString("Could not execute %1: %2")
                    .arg(executable.c_str())
                    .arg(SysError(err).Message())
            );
        }
        qApp->quit();
    }
    
    void onTimeout()
    {
        qApp->quit();
    }
    
    QTimer mTimer;
};

int main(int argc, char **argv)
{
    QtApplication app(argc, argv);
    FileOpenHandler handler;
    app.connect(&app, &QtApplication::FileOpen, &handler, &FileOpenHandler::onFileOpen);
    return app.Run();
}
