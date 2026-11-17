/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionQuit;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionBCI2000_Help;
    QAction *actionAbout;
    QAction *actionTo_Applications;
    QAction *actionTo_Signal_Processing;
    QAction *actionTo_Signal_Source;
    QAction *actionTop;
    QAction *actionBottom;
    QAction *actionUp;
    QAction *actionDown;
    QAction *actionHide;
    QAction *actionCreate_Batch_File;
    QAction *actionReset_View;
    QAction *actionReset_List_of_Modules;
    QWidget *centralWidget;
    QListWidget *applicationList;
    QListWidget *sigprocList;
    QPushButton *resetSelectionBtn;
    QListWidget *sourceList;
    QTextEdit *statusEdit;
    QPushButton *launchButton;
    QPushButton *addParmfileBtn;
    QPushButton *removeParmfileBtn;
    QLineEdit *subjectSessionEdit;
    QPushButton *chooseDirBtn;
    QComboBox *fileFormatComboBox;
    QListWidget *paramfilesList;
    QLineEdit *saveDirEdit;
    QLineEdit *subjectNameEdit;
    QPlainTextEdit *scriptingCommandsEdit;
    QTreeWidget *extensionsList;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuHelp;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(746, 620);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionCut = new QAction(MainWindow);
        actionCut->setObjectName(QString::fromUtf8("actionCut"));
        actionCut->setEnabled(false);
        actionCopy = new QAction(MainWindow);
        actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
        actionCopy->setEnabled(false);
        actionPaste = new QAction(MainWindow);
        actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
        actionPaste->setEnabled(false);
        actionBCI2000_Help = new QAction(MainWindow);
        actionBCI2000_Help->setObjectName(QString::fromUtf8("actionBCI2000_Help"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionTo_Applications = new QAction(MainWindow);
        actionTo_Applications->setObjectName(QString::fromUtf8("actionTo_Applications"));
        actionTo_Signal_Processing = new QAction(MainWindow);
        actionTo_Signal_Processing->setObjectName(QString::fromUtf8("actionTo_Signal_Processing"));
        actionTo_Signal_Source = new QAction(MainWindow);
        actionTo_Signal_Source->setObjectName(QString::fromUtf8("actionTo_Signal_Source"));
        actionTop = new QAction(MainWindow);
        actionTop->setObjectName(QString::fromUtf8("actionTop"));
        actionBottom = new QAction(MainWindow);
        actionBottom->setObjectName(QString::fromUtf8("actionBottom"));
        actionUp = new QAction(MainWindow);
        actionUp->setObjectName(QString::fromUtf8("actionUp"));
        actionDown = new QAction(MainWindow);
        actionDown->setObjectName(QString::fromUtf8("actionDown"));
        actionHide = new QAction(MainWindow);
        actionHide->setObjectName(QString::fromUtf8("actionHide"));
        actionCreate_Batch_File = new QAction(MainWindow);
        actionCreate_Batch_File->setObjectName(QString::fromUtf8("actionCreate_Batch_File"));
        actionReset_View = new QAction(MainWindow);
        actionReset_View->setObjectName(QString::fromUtf8("actionReset_View"));
        actionReset_List_of_Modules = new QAction(MainWindow);
        actionReset_List_of_Modules->setObjectName(QString::fromUtf8("actionReset_List_of_Modules"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        applicationList = new QListWidget(centralWidget);
        applicationList->setObjectName(QString::fromUtf8("applicationList"));
        applicationList->setGeometry(QRect(370, 30, 131, 131));
        applicationList->setContextMenuPolicy(Qt::ActionsContextMenu);
        applicationList->setSortingEnabled(false);
        sigprocList = new QListWidget(centralWidget);
        sigprocList->setObjectName(QString::fromUtf8("sigprocList"));
        sigprocList->setGeometry(QRect(220, 30, 131, 131));
        sigprocList->setContextMenuPolicy(Qt::ActionsContextMenu);
        sigprocList->setSortingEnabled(false);
        resetSelectionBtn = new QPushButton(centralWidget);
        resetSelectionBtn->setObjectName(QString::fromUtf8("resetSelectionBtn"));
        resetSelectionBtn->setGeometry(QRect(250, 160, 91, 24));
        sourceList = new QListWidget(centralWidget);
        sourceList->setObjectName(QString::fromUtf8("sourceList"));
        sourceList->setGeometry(QRect(50, 30, 131, 131));
        sourceList->setContextMenuPolicy(Qt::ActionsContextMenu);
        sourceList->setSortingEnabled(false);
        statusEdit = new QTextEdit(centralWidget);
        statusEdit->setObjectName(QString::fromUtf8("statusEdit"));
        statusEdit->setGeometry(QRect(490, 220, 104, 64));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier"));
        statusEdit->setFont(font);
        launchButton = new QPushButton(centralWidget);
        launchButton->setObjectName(QString::fromUtf8("launchButton"));
        launchButton->setGeometry(QRect(500, 300, 91, 31));
        addParmfileBtn = new QPushButton(centralWidget);
        addParmfileBtn->setObjectName(QString::fromUtf8("addParmfileBtn"));
        addParmfileBtn->setGeometry(QRect(300, 210, 31, 24));
        removeParmfileBtn = new QPushButton(centralWidget);
        removeParmfileBtn->setObjectName(QString::fromUtf8("removeParmfileBtn"));
        removeParmfileBtn->setGeometry(QRect(300, 250, 31, 24));
        subjectSessionEdit = new QLineEdit(centralWidget);
        subjectSessionEdit->setObjectName(QString::fromUtf8("subjectSessionEdit"));
        subjectSessionEdit->setGeometry(QRect(230, 350, 113, 20));
        chooseDirBtn = new QPushButton(centralWidget);
        chooseDirBtn->setObjectName(QString::fromUtf8("chooseDirBtn"));
        chooseDirBtn->setGeometry(QRect(260, 320, 75, 24));
        fileFormatComboBox = new QComboBox(centralWidget);
        fileFormatComboBox->setObjectName(QString::fromUtf8("fileFormatComboBox"));
        fileFormatComboBox->setGeometry(QRect(90, 290, 231, 22));
        paramfilesList = new QListWidget(centralWidget);
        paramfilesList->setObjectName(QString::fromUtf8("paramfilesList"));
        paramfilesList->setGeometry(QRect(50, 210, 231, 71));
        paramfilesList->setContextMenuPolicy(Qt::ActionsContextMenu);
        paramfilesList->setTextElideMode(Qt::ElideMiddle);
        saveDirEdit = new QLineEdit(centralWidget);
        saveDirEdit->setObjectName(QString::fromUtf8("saveDirEdit"));
        saveDirEdit->setGeometry(QRect(50, 320, 151, 20));
        subjectNameEdit = new QLineEdit(centralWidget);
        subjectNameEdit->setObjectName(QString::fromUtf8("subjectNameEdit"));
        subjectNameEdit->setGeometry(QRect(60, 350, 113, 20));
        scriptingCommandsEdit = new QPlainTextEdit(centralWidget);
        scriptingCommandsEdit->setObjectName(QString::fromUtf8("scriptingCommandsEdit"));
        scriptingCommandsEdit->setGeometry(QRect(450, 440, 104, 64));
        scriptingCommandsEdit->setFont(font);
        scriptingCommandsEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scriptingCommandsEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scriptingCommandsEdit->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
        scriptingCommandsEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
        extensionsList = new QTreeWidget(centralWidget);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        extensionsList->setHeaderItem(__qtreewidgetitem);
        extensionsList->setObjectName(QString::fromUtf8("extensionsList"));
        extensionsList->setGeometry(QRect(80, 400, 241, 161));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 746, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionReset_View);
        menuFile->addAction(actionReset_List_of_Modules);
        menuFile->addSeparator();
        menuFile->addAction(actionCreate_Batch_File);
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuHelp->addAction(actionBCI2000_Help);
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionQuit->setText(QCoreApplication::translate("MainWindow", "Quit", nullptr));
#if QT_CONFIG(shortcut)
        actionQuit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCut->setText(QCoreApplication::translate("MainWindow", "Cut", nullptr));
#if QT_CONFIG(shortcut)
        actionCut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCopy->setText(QCoreApplication::translate("MainWindow", "Copy", nullptr));
#if QT_CONFIG(shortcut)
        actionCopy->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPaste->setText(QCoreApplication::translate("MainWindow", "Paste", nullptr));
#if QT_CONFIG(shortcut)
        actionPaste->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+V, Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionBCI2000_Help->setText(QCoreApplication::translate("MainWindow", "BCI2000 Help", nullptr));
        actionAbout->setText(QCoreApplication::translate("MainWindow", "About ...", nullptr));
        actionTo_Applications->setText(QCoreApplication::translate("MainWindow", "to Applications", nullptr));
        actionTo_Signal_Processing->setText(QCoreApplication::translate("MainWindow", "to Signal Processing", nullptr));
        actionTo_Signal_Source->setText(QCoreApplication::translate("MainWindow", "to Signal Source", nullptr));
        actionTop->setText(QCoreApplication::translate("MainWindow", "Move to Top", nullptr));
        actionBottom->setText(QCoreApplication::translate("MainWindow", "Move to Bottom", nullptr));
        actionUp->setText(QCoreApplication::translate("MainWindow", "Move Up", nullptr));
        actionDown->setText(QCoreApplication::translate("MainWindow", "Move Down", nullptr));
        actionHide->setText(QCoreApplication::translate("MainWindow", "Hide from List", nullptr));
        actionCreate_Batch_File->setText(QCoreApplication::translate("MainWindow", "Create Batch File ...", nullptr));
#if QT_CONFIG(shortcut)
        actionCreate_Batch_File->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionReset_View->setText(QCoreApplication::translate("MainWindow", "Reset View", nullptr));
#if QT_CONFIG(shortcut)
        actionReset_View->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+R", nullptr));
#endif // QT_CONFIG(shortcut)
        actionReset_List_of_Modules->setText(QCoreApplication::translate("MainWindow", "Reset List of Modules", nullptr));
        resetSelectionBtn->setText(QCoreApplication::translate("MainWindow", "Reset Selection", nullptr));
        launchButton->setText(QCoreApplication::translate("MainWindow", "Launch", nullptr));
        addParmfileBtn->setText(QCoreApplication::translate("MainWindow", "Add...", nullptr));
        removeParmfileBtn->setText(QCoreApplication::translate("MainWindow", "Remove", nullptr));
        chooseDirBtn->setText(QCoreApplication::translate("MainWindow", "Choose...", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
