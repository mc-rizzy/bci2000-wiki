//////////////////////////////////////////////////////////////////////////////
// $Id: MainWindow.h 9216 2026-01-01 14:20:40Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: BCI2000Export main window
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void ConvertFiles(const QStringList&);

  private slots:
    void on_FileOpen();
    void on_FileQuit();
    void on_HelpBCI2000Help();
    void on_HelpAbout();

  private:
    void EnableAll();
    void DisableAll();
    void SaveSettings() const;
    void ReadSettings();

  private:
    struct StatesListWidget : public QListWidget
    {
        void HandleFiles(const QStringList&);
        void dragEnterEvent(QDragEnterEvent*) override;
        void dragMoveEvent(QDragMoveEvent*) override;
        void dropEvent(QDropEvent*) override;

        void keyPressEvent(QKeyEvent *);
    } *mpImportStates;

    struct ChannelsTextEdit : public QTextEdit
    {
        void HandleFiles(const QStringList&);
        void dragEnterEvent(QDragEnterEvent*) override;
        void dropEvent(QDropEvent*) override;
    } *mpChannelNames;

    struct DropFilesLabel : public QLabel
    {
        DropFilesLabel(const QString&, MainWindow*);
        void HandleFiles(const QStringList&);
        void dragEnterEvent(QDragEnterEvent*) override;
        void dropEvent(QDropEvent*) override;
        MainWindow* mpParent;
    } *mpDropArea;

    QComboBox *mpOutputFormat;
    QLabel *mpStatusLabel;
};

#endif // MAINWINDOW_H
