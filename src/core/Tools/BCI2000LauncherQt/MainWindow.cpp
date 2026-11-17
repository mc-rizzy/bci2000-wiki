////////////////////////////////////////////////////////////////////////////////
// $Id: MainWindow.cpp 9574 2026-09-24 12:32:43Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The main window of the BCI2000Launcher tool.
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
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AboutBox.h"
#include "EncodedString.h"
#include "ExecutableHelp.h"
#include "Settings.h"
#include "SysError.h"
#include "Resources.h"
#include "ProcessUtils.h"
#include "BCI2000Remote.h"
#include "ClickableLabel.h"
#include "EnvVariable.h"

#include <QtWidgets>
#include <sstream>
#include <string>
#include <thread>

#if _WIN32
#include <Windows.h>
#endif

static const int cIconSize = 32;
static const int cHelpIconSize = 16;
static const QString cSourceDefault = "SignalGenerator";
static const QString cSigProcDefault = "DummySignalProcessing";
static const QString cAppDefault = "DummyApplication";

static std::string FindBCI2000Tool(const std::string& name)
{
    std::string path = FileUtils::InstallationDirectory();
    path += "../tools/" + name + "/" + name;
#if __APPLE__
    path += ".app/Contents/MacOS/" + name;
#endif
    return path;
}

static std::string EncodeParamValue(const char* value)
{
    std::ostringstream oss;
    EncodedString(value).InsertInto(oss, ";");
    return oss.str();
}

static std::string SubstituteVars(const std::map<std::string, std::string>& vars, const std::string& text)
{
    std::string result = text;
    for (const auto& var : vars) {
        std::string ref = "%" + var.first + "%";
        size_t pos = 0;
        while ((pos = result.find(ref)) != std::string::npos)
            result = result.substr(0, pos) + var.second + result.substr(pos + ref.length());
    }
    return result;
}

// Without a custom style, the tab control would draw its labels vertically when positioned west (left)
class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
        const QSize& size, const QWidget* widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab) {
            s.transpose();
        }
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
    {
        if (element == CE_TabBarTabLabel) {
            if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
                QStyleOptionTab opt(*tab);
                opt.shape = QTabBar::RoundedNorth;
                QProxyStyle::drawControl(element, &opt, painter, widget);
                return;
            }
        }
        QProxyStyle::drawControl(element, option, painter, widget);
    }
};

class CustomGroupBox : public QWidget
{
  public:
    CustomGroupBox(const QString& title = "")
    {
        mpLabel = new QLabel;
        mpTitleLayout = new QHBoxLayout;
        mpTitleLayout->setContentsMargins(0, 0, 0, 0);
        mpTitleLayout->addWidget(mpLabel, 0, Qt::AlignBottom);
        mpTitleWidget = new QWidget;
        mpTitleWidget->setLayout(mpTitleLayout);

        mpGroupBox = new QGroupBox;
        mpLayout = new QVBoxLayout;
        mpLayout->addWidget(mpTitleWidget, 0, Qt::AlignBottom);
        mpLayout->addWidget(mpGroupBox);
        mpLayout->setContentsMargins(0, 0, 0, 0);
        mpLayout->setSpacing(0);
        setLayout(mpLayout);
        setTitle(title);
    }
    void setTitle(const QString& title)
    {
        mpLabel->setText(title);
    }
    QString title() const
    {
        return mpLabel->text();
    }
    QHBoxLayout* titleLayout()
    {
        return mpTitleLayout;
    }
    void setContentLayout(QLayout* pLayout)
    {
        mpGroupBox->setLayout(pLayout);
    }
    QLayout* contentLayout()
    {
        return mpGroupBox->layout();
    }

  private:
    QLabel* mpLabel;
    QGroupBox* mpGroupBox;
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpTitleLayout;
    QWidget* mpTitleWidget;
};

// MainWindow
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    EnvVariable::Set("OMP_WAIT_POLICY", "PASSIVE");

    ui->setupUi(this);

    this->setWindowTitle(QApplication::applicationName());
    SetupProgramsGroup();
    SetupSelectionGroup();
    SetupExtensionsGroup();
    SetupParametersGroup();
    SetupScriptingCommandsGroup();
    SetupStatusGroup();
    SetupGlobalLayout();
    SetupLists();

    mDefaultStateVertical1 = mpVerticalSplitter1->saveState();
    mDefaultStateVertical2 = mpVerticalSplitter2->saveState();
    mDefaultStateHorizontal = mpHorizontalSplitter->saveState();

    ReadSettings();

    QStringList programs = ScanForPrograms();
    DistributePrograms(programs);
    UpdateExtensionLabels();
    mpTabWidget->setCurrentIndex(0);
    onProgramUpdate(ui->sourceList);
}

MainWindow::~MainWindow()
{
    SaveSettings();
    delete ui;
}

void MainWindow::SetupProgramsGroup()
{
    mpProgramsGroup = new CustomGroupBox(tr("BCI2000 Modules"));
    mpProgramsGroup->titleLayout()->insertWidget(1, CreateHelpIcon("Technical_Reference:System_Design"));
    mpProgramsGroup->titleLayout()->insertStretch(2);
    mpProgramsGroup->setContentLayout(new QHBoxLayout);

    mpTabWidget = new QTabWidget;
    mpTabWidget->setIconSize(QSize(cIconSize, cIconSize));
#if !__APPLE__
    mpTabWidget->setTabPosition(QTabWidget::West);
    mpTabWidget->tabBar()->setStyle(new CustomTabStyle);
#endif
    SetupSingleTab(ui->sourceList, &bci::Resources::SignalScreen_png, "Signal Source");
    SetupSingleTab(ui->sigprocList, &bci::Resources::CogwheelScreen_png, "Signal Processing");
    SetupSingleTab(ui->applicationList, &bci::Resources::PresentationScreen_png, "Application");
    mpProgramsGroup->contentLayout()->addWidget(mpTabWidget);

    connect(mpTabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

void MainWindow::SetupSingleTab(QListWidget* pContent, const bci::Resource* pIcon, const QString& name)
{
    if (pIcon) {
        QPixmap pixmap;
        pixmap.loadFromData(QByteArray(pIcon->data, pIcon->length));
        mpTabWidget->addTab(pContent, pixmap, name);
    }
    else {
        mpTabWidget->addTab(pContent, name);
    }
    pContent->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(pContent, &QListWidget::itemSelectionChanged, this, &MainWindow::onProgramSelectionChanged);
    connect(pContent, &QListWidget::itemDoubleClicked, this, &MainWindow::onProgramDoubleClick);
}

void MainWindow::SetupSelectionGroup()
{
    // Setup layouts in the selection group
    QVBoxLayout *pVBoxProg = new QVBoxLayout;
    QGridLayout *pGridLayout = new QGridLayout;

    QPixmap pixmap;
    QLabel* pIconLabel = new QLabel;
    pixmap.loadFromData(QByteArray(Resources::SignalScreen_png.data, Resources::SignalScreen_png.length));
    pIconLabel->setPixmap(pixmap.scaled(cIconSize, cIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pIconLabel->setMaximumSize(pIconLabel->minimumSizeHint());
    pGridLayout->addWidget(pIconLabel, 0, 0);
    QLabel* pKindLabel = new QLabel("Signal Source:");
    pGridLayout->addWidget(pKindLabel, 0, 1);

    mpSourceSelected = new QLabel;
    QFont boldFont = mpSourceSelected->font();
    boldFont.setBold(true);
    mpSourceSelected->setFont(boldFont);
    pGridLayout->addWidget(mpSourceSelected, 0, 2);
    ClearSelectionLabel(mpSourceSelected);

    mpSourceExtensions = new QLabel;
    QFont italicFont = mpSourceExtensions->font();
    italicFont.setItalic(true);
    mpSourceExtensions->setFont(italicFont);
    pGridLayout->addWidget(mpSourceExtensions, 0, 3);

    pIconLabel = new QLabel;
    pixmap.loadFromData(QByteArray(Resources::CogwheelScreen_png.data, Resources::CogwheelScreen_png.length));
    pIconLabel->setPixmap(pixmap.scaled(cIconSize, cIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pIconLabel->setMaximumSize(pIconLabel->minimumSizeHint());
    pGridLayout->addWidget(pIconLabel, 1, 0);
    pGridLayout->addWidget(new QLabel("Signal Processing:"), 1, 1);

    mpSigProcSelected = new QLabel;
    mpSigProcSelected->setFont(boldFont);
    pGridLayout->addWidget(mpSigProcSelected, 1, 2);
    ClearSelectionLabel(mpSigProcSelected);

    mpSigProcExtensions = new QLabel;
    mpSigProcExtensions->setFont(italicFont);
    pGridLayout->addWidget(mpSigProcExtensions, 1, 3);

    pIconLabel = new QLabel;
    pixmap.loadFromData(QByteArray(Resources::PresentationScreen_png.data, Resources::PresentationScreen_png.length));
    pIconLabel->setPixmap(pixmap.scaled(cIconSize, cIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pIconLabel->setMaximumSize(pIconLabel->minimumSizeHint());
    pGridLayout->addWidget(pIconLabel, 2, 0);
    pGridLayout->addWidget(new QLabel("Application:"), 2, 1);

    mpAppSelected = new QLabel;
    mpAppSelected->setFont(boldFont);
    pGridLayout->addWidget(mpAppSelected, 2, 2);
    ClearSelectionLabel(mpAppSelected);

    mpAppExtensions = new QLabel;
    mpAppExtensions->setFont(italicFont);
    pGridLayout->addWidget(mpAppExtensions, 2, 3);

    pVBoxProg->addLayout(pGridLayout);
    pVBoxProg->addWidget(ui->resetSelectionBtn);

    mpSelectionGroup = new CustomGroupBox(tr("Selection"));
    mpSelectionGroup->setContentLayout(pVBoxProg);
}

void MainWindow::SetupExtensionsGroup()
{
    QLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(ui->extensionsList);
    mpExtensionsGroup = new CustomGroupBox;
    SetExtensionsGroupTitle("");
    mpExtensionsGroup->titleLayout()->insertWidget(1, CreateHelpIcon("Contributions:Extensions"));
    mpExtensionsGroup->titleLayout()->insertStretch(2);
    mpExtensionsGroup->setContentLayout(pLayout);
    ui->extensionsList->setHeaderHidden(true);
    connect(ui->extensionsList, &QTreeWidget::itemChanged, this, &MainWindow::onExtensionItemChanged);
    connect(ui->extensionsList, &QTreeWidget::itemCollapsed, this, &MainWindow::onExtensionCollapsedChanged);
    connect(ui->extensionsList, &QTreeWidget::itemExpanded, this, &MainWindow::onExtensionCollapsedChanged);
}

void MainWindow::SetupScriptingCommandsGroup()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(ui->scriptingCommandsEdit);
    mpScriptingCommandsGroup = new CustomGroupBox(tr("Scripting Commands"));
    mpScriptingCommandsGroup->titleLayout()->insertWidget(1, CreateHelpIcon("User_Reference:Operator_Module_Scripting"));
    mpScriptingCommandsGroup->titleLayout()->insertStretch(2);
    mpScriptingCommandsGroup->setContentLayout(pLayout);
}

void MainWindow::SetupParametersGroup()
{
    // Setup layouts in the parameters group
    QVBoxLayout *pVBoxParm = new QVBoxLayout, *pVBox1 = new QVBoxLayout, *pVBox2 = new QVBoxLayout;
    QHBoxLayout *pHBox1 = new QHBoxLayout, *pHBox2 = new QHBoxLayout, *pHBox3 = new QHBoxLayout,
                *pHBox11 = new QHBoxLayout;

    pVBox2->addWidget(ui->addParmfileBtn);
    pVBox2->addWidget(ui->removeParmfileBtn);
    pVBox2->addStretch();

    pHBox1->addWidget(ui->paramfilesList);
    pHBox1->addLayout(pVBox2);

    pVBox1->addWidget(new QLabel(tr("Parameter Files")));
    pVBox1->addLayout(pHBox1);

    pHBox11->addWidget(new QLabel(tr("Data File Format")));
    ui->fileFormatComboBox->addItem(tr("BCI2000 native"), "BCI2000");
    ui->fileFormatComboBox->addItem("EDF", "EDF");
    ui->fileFormatComboBox->addItem("GDF", "GDF");
    ui->fileFormatComboBox->addItem(tr("no recording"), "Null");
    pHBox11->addWidget(ui->fileFormatComboBox);
    pHBox11->addStretch();

    pHBox2->addWidget(new QLabel(tr("Save Directory")));
    pHBox2->addWidget(ui->saveDirEdit);
    pHBox2->addWidget(ui->chooseDirBtn);

    pHBox3->addWidget(new QLabel(tr("Subject Name")));
    pHBox3->addWidget(ui->subjectNameEdit);
    pHBox3->addWidget(new QLabel(tr("Session #")));
    pHBox3->addWidget(ui->subjectSessionEdit);

    pVBoxParm->addLayout(pVBox1);
    pVBoxParm->addLayout(pHBox2);
    pVBoxParm->addLayout(pHBox3);
    pVBoxParm->addLayout(pHBox11);
    mpParametersGroup = new CustomGroupBox(tr("Parameters"));
    mpParametersGroup->titleLayout()->insertWidget(1, CreateHelpIcon("Technical_Reference:Parameter_File"));
    mpParametersGroup->titleLayout()->insertStretch(2);
    mpParametersGroup->setContentLayout(pVBoxParm);
}

void MainWindow::SetupStatusGroup()
{
    // Setup layouts in the status group
    QVBoxLayout *pVBoxStatus = new QVBoxLayout;
    pVBoxStatus->addWidget(ui->statusEdit);
    pVBoxStatus->addWidget(ui->launchButton);
    mpStatusGroup = new CustomGroupBox(tr("Status"));
    mpStatusGroup->setContentLayout(pVBoxStatus);
    ui->statusEdit->setReadOnly(true);
}

void MainWindow::SetupGlobalLayout()
{
    mpHorizontalSplitter = new QSplitter(Qt::Horizontal);
    mpHorizontalSplitter->setChildrenCollapsible(false);
    mpVerticalSplitter1 = new QSplitter(Qt::Vertical);
    mpVerticalSplitter1->setChildrenCollapsible(false);
    mpVerticalSplitter1->addWidget(mpProgramsGroup);

    QVBoxLayout* pLowerPaneLayout = new QVBoxLayout;
    pLowerPaneLayout->setContentsMargins(0, 0, 0, 0);
    pLowerPaneLayout->addWidget(mpSelectionGroup);
    pLowerPaneLayout->addWidget(mpStatusGroup);
    QWidget* pLowerPane = new QWidget;
    pLowerPane->setLayout(pLowerPaneLayout);
    mpVerticalSplitter1->addWidget(pLowerPane);

    mpVerticalSplitter1->setSizes({2000, 2000});
    mpHorizontalSplitter->addWidget(mpVerticalSplitter1);

    mpVerticalSplitter2 = new QSplitter(Qt::Vertical);
    mpVerticalSplitter2->setChildrenCollapsible(false);
    mpHorizontalSplitter->addWidget(mpVerticalSplitter2);
    mpHorizontalSplitter->setSizes({2000, 2000});

    mpVerticalSplitter2->addWidget(mpExtensionsGroup);
    mpVerticalSplitter2->addWidget(mpScriptingCommandsGroup);
    mpVerticalSplitter2->addWidget(mpParametersGroup);
    mpVerticalSplitter2->setSizes({2000, 2000, 2000});

    QLayout* pLayout = new QHBoxLayout;
    pLayout->addWidget(mpHorizontalSplitter);
    centralWidget()->setLayout(pLayout);
}

void MainWindow::SetupLists()
{
    for (auto pList : {ui->sourceList, ui->sigprocList, ui->applicationList, ui->paramfilesList})
        SetupList(pList);
}

void MainWindow::SetupList(QListWidget* pList)
{
    pList->setDragDropMode(QAbstractItemView::InternalMove);
    pList->addAction(ui->actionUp);
    pList->addAction(ui->actionDown);
    if (pList != ui->paramfilesList) {
        QAction* pSeparator = new QAction(this);
        pSeparator->setSeparator(true);
        pList->addAction(pSeparator);
        pList->addAction(ui->actionTop);
        pList->addAction(ui->actionBottom);
        pSeparator = new QAction(this);
        pSeparator->setSeparator(true);
        pList->addAction(pSeparator);
        pList->addAction(ui->actionHide);
    }
}

void MainWindow::CreateProgramItem(QListWidget* pList, const QString& name)
{
    QListWidgetItem* pItem = new QListWidgetItem(name, pList);
    StyleItem(pItem);
}

void MainWindow::StyleItem(QListWidgetItem* pItem)
{
    if (mBrokenModules.contains(pItem->text())) {
        pItem->setForeground(Qt::gray);
        QFont font = pItem->font();
        font.setItalic(true);
        pItem->setFont(font);
        pItem->setToolTip(QString(
            "Cannot obtain information from \"%1\".\n"
            "It may depend on libraries that are not installed."
        ).arg(pItem->text()));
    }
}

QStringList MainWindow::ScanForPrograms()
{
    QDir dir(FileUtils::InstallationDirectory().c_str());
    dir.setFilter(QDir::Files | QDir::Executable);
    dir.setSorting(QDir::Name | QDir::IgnoreCase);
    QStringList entries = dir.entryList();
#if __APPLE__
    dir.setFilter(QDir::Dirs);
    dir.setNameFilters(QStringList() << "*.app");
    entries.append(dir.entryList());
#endif
    QStringList programs;
    for (const auto& entry : std::as_const(entries))
        programs.append(QFileInfo(entry).completeBaseName());
    // Remove the program itself.
    programs.removeOne(QFileInfo(QApplication::applicationFilePath()).completeBaseName());
    // Remove all operator modules.
    for (const auto& prog : mInventory[bci::Inventory::Operator])
        programs.removeOne(QString::fromLocal8Bit(prog.c_str()));
    // Remove all tools.
    for (const auto& prog : mInventory[bci::Inventory::Tool])
        programs.removeOne(QString::fromLocal8Bit(prog.c_str()));
    // Remove all helpers.
    for (const auto& prog : mInventory[bci::Inventory::Helper])
      programs.removeOne(QString::fromLocal8Bit(prog.c_str()));
    // Remove disabled modules.
    for (const auto& prog : std::as_const(mHiddenModules))
      programs.removeOne(prog);
    return programs;
}

void MainWindow::DistributePrograms(const QStringList &inList)
{
    // Build a list of all programs known by the application.
    QStringList knownPrograms;
    for (int i = 0; i < ui->sourceList->count(); ++i)
        knownPrograms.append(ui->sourceList->item(i)->text());
    for (int i = 0; i < ui->sigprocList->count(); ++i)
        knownPrograms.append(ui->sigprocList->item(i)->text());
    for (int i = 0; i < ui->applicationList->count(); ++i)
        knownPrograms.append(ui->applicationList->item(i)->text());
    QStringList unknownPrograms;
    for (const QString& program : inList)
    {
        if (knownPrograms.contains(program, Qt::CaseSensitive))
            knownPrograms.removeOne(program);
        else
            unknownPrograms.append(program);
    }
    for (const QString& program : knownPrograms)
        ClearProgramEntries(program);
    for (const QString& program : unknownPrograms)
        IntroduceNewProgram(program);
}

void MainWindow::ClearProgramEntries(const QString &inProgram)
{ // Remove a program's entries from all program lists.
    QList<QListWidgetItem *> items = ui->sourceList->findItems(inProgram, Qt::MatchExactly);
    items.append(ui->sigprocList->findItems(inProgram, Qt::MatchExactly));
    items.append(ui->applicationList->findItems(inProgram, Qt::MatchExactly));
    for (QListWidgetItem *item : std::as_const(items))
        delete item;
}

void MainWindow::IntroduceNewProgram(const QString &inProgram)
{ // Introduce a new program into the programs group.
    // Check against the lists of known modules first.
    // If not found there, add to the "others" list.
    const bci::Inventory::mapped_type &sources = mInventory[bci::Inventory::SignalSource];
    if (sources.find(inProgram.toLocal8Bit().constData()) != sources.end())
    {
        CreateProgramItem(ui->sourceList, inProgram);
        return;
    }

    const bci::Inventory::mapped_type &sigprocs = mInventory[bci::Inventory::SignalProcessing];
    if (sigprocs.find(inProgram.toLocal8Bit().constData()) != sigprocs.end())
    {
        CreateProgramItem(ui->sigprocList, inProgram);
        return;
    }

    const bci::Inventory::mapped_type &apps = mInventory[bci::Inventory::Application];
    if (apps.find(inProgram.toLocal8Bit().constData()) != apps.end())
    {
        CreateProgramItem(ui->applicationList, inProgram);
        return;
    }
}

QStringList MainWindow::GetExtensions(const QString& program, bool& failed) const
{
    failed = false;
    QProcess process;
    QString executable = QString::fromLocal8Bit(FileUtils::InstallationDirectory().c_str()) + program;
#if __APPLE__
    if (!QFile::exists(executable))
        executable += ".app/Contents/MacOS/" + program;
#endif
    process.setProgram(executable);
    process.setWorkingDirectory(QString::fromLocal8Bit(FileUtils::InstallationDirectory().c_str()));
    process.setArguments({"--Extensions"});
#if _WIN32
    // Don't show "missing DLL" dialog
    ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
#endif
    process.start();
    process.waitForFinished(1000);
    if (process.state() == QProcess::Running) {
        failed = true;
        process.kill();
        process.waitForFinished();
    }

    QByteArray data = process.readAllStandardOutput();
    QString stringdata = QString::fromUtf8(data.constData());
    stringdata.replace("\r\n", "\n");
    QStringList list = stringdata.split("\n", Qt::SkipEmptyParts);
    if (!list.isEmpty())
        list.pop_front();
    return list;
}

void MainWindow::ObtainExtensionsThread(QString module, QString category)
{
    bool failed = false;
    QStringList extensions = GetExtensions(module, failed);
    if (failed) {
        QMetaObject::invokeMethod(this, "onExtensionsFailed", Q_ARG(QString, module));
    }
    else {
        QMetaObject::invokeMethod(this, "onExtensionsObtained", Q_ARG(QStringList, extensions), Q_ARG(QString, category), Q_ARG(QString, module));
    }
    QThread::currentThread()->deleteLater();
}

QString MainWindow::ExtensionArgs(const QString& category) const
{
    QString args;
    for (const auto& enabler : mExtensionEnablerState)
        if (enabler.second.checked && enabler.second.category == category)
            args += " --" + enabler.first + "=1";
    return args;
}

void MainWindow::SetExtensionsGroupTitle(const QString& moduleName) const
{
    QString title = tr("Extensions");
    if (!moduleName.isEmpty())
        title += ": " + moduleName;
    mpExtensionsGroup->setTitle(title);
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::onTabChanged()
{
    auto* pList = dynamic_cast<QListWidget*>(mpTabWidget->currentWidget());
    if (pList)
        onProgramUpdate(pList);
}

void MainWindow::onProgramDoubleClick()
{
    auto pSender = dynamic_cast<QListWidget*>(sender());
    if (!pSender)
        return;

    QLabel* pLabel = nullptr;
    if (pSender == ui->sourceList) {
        pLabel = mpSourceSelected;
    }
    else if (pSender == ui->sigprocList) {
        pLabel = mpSigProcSelected;
    }
    else if (pSender == ui->applicationList) {
        pLabel = mpAppSelected;
    }
    auto pItem = pSender->currentItem();
    if (pLabel) {
        pLabel->setText(pItem->text());
    }
}

void MainWindow::onProgramSelectionChanged()
{
    auto pOrigin = dynamic_cast<QListWidget*>(sender());
    if (pOrigin)
        onProgramUpdate(pOrigin);
}

void MainWindow::onProgramUpdate(QListWidget* pOrigin)
{
    QString category = "other";
    if (pOrigin == ui->sourceList) {
        category = "source";
    }
    else if (pOrigin == ui->sigprocList) {
        category = "sigproc";
    }
    else if (pOrigin == ui->applicationList) {
        category = "app";
    }
    else {
        return;
    }

    auto selectedItems = pOrigin->selectedItems();
    if (selectedItems.isEmpty())
        return;
    QListWidgetItem* pSelectedItem = selectedItems.front();
    QString moduleName = pSelectedItem->text();
    if (!mBrokenModules.contains(moduleName)) {
        if (mExtensionsGroupLock.try_lock()) {
            SetExtensionsGroupTitle(moduleName);
            QThread* pThread = QThread::create([this, moduleName, category]() {ObtainExtensionsThread(moduleName, category); });
            pThread->start();
        }
    }
}

void MainWindow::onHelpIconClicked()
{
    auto pSender = sender();
    if (!pSender)
        return;

    const QVariant& prop = pSender->property("HelpTarget");
    if (!prop.isValid())
        return;

    QString helpTarget = prop.toString();
    ExecutableHelp().DisplayWikiPage(helpTarget.toLocal8Bit().constData());
}

void MainWindow::onExtensionsObtained(QStringList extensions, const QString& category, const QString& moduleName)
{
    ui->extensionsList->clear();
    extensions.sort();
    for (const auto& extension : extensions) {
        QStringList enablers = extension.split("\t");
        QTreeWidgetItem* pTopItem = new QTreeWidgetItem({ enablers[0] });
        for (int i = 1; i < enablers.size(); ++i) {
            auto pItem = new QTreeWidgetItem(pTopItem, { enablers[i] });
            pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable);
            bool checked = mExtensionEnablerState[enablers[i]].checked;
            mExtensionEnablerState[enablers[i]].category = category;
            mExtensionEnablerState[enablers[i]].extension = enablers[0];
            pItem->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
            pItem->setData(0, Qt::CheckStateRole, checked ? Qt::Checked : Qt::Unchecked);
        }
        ui->extensionsList->addTopLevelItem(pTopItem);
        pTopItem->setExpanded(!mExtensionCollapsedState[pTopItem->text(0)]);
    }
    SetExtensionsGroupTitle(moduleName);
    mExtensionsGroupLock.unlock();
}

void MainWindow::onExtensionsFailed(const QString& moduleName)
{
    ui->extensionsList->clear();
    for (auto* pList : { ui->sourceList, ui->sigprocList, ui->applicationList }) {
        auto items = pList->findItems(moduleName, Qt::MatchExactly);
        for (auto pItem : std::as_const(items)) {
            mBrokenModules.push_back(pItem->text());
            StyleItem(pItem);
        }
    }
    SetExtensionsGroupTitle("");
    mExtensionsGroupLock.unlock();
}

void MainWindow::onExtensionItemChanged(QTreeWidgetItem* pItem)
{
    int value = pItem->data(0, Qt::CheckStateRole).toInt();
    mExtensionEnablerState[pItem->text(0)].checked = value > 0;
    UpdateExtensionLabels();
}

void MainWindow::onExtensionCollapsedChanged(QTreeWidgetItem* pItem)
{
    bool expanded = pItem->isExpanded();
    mExtensionCollapsedState[pItem->text(0)] = !expanded;
}

void MainWindow::on_actionAbout_triggered()
{
    AboutBox().SetApplicationName(QApplication::applicationName().toLocal8Bit().constData()).Display();
}

void MainWindow::on_actionReset_View_triggered()
{
    mpVerticalSplitter1->restoreState(mDefaultStateVertical1);
    mpVerticalSplitter2->restoreState(mDefaultStateVertical2);
    mpHorizontalSplitter->restoreState(mDefaultStateHorizontal);
}

void MainWindow::on_actionReset_List_of_Modules_triggered()
{
    mBrokenModules.clear();
    mHiddenModules.clear();
    for (auto pList : {ui->sourceList, ui->sigprocList, ui->applicationList})
        pList->clear();
    QStringList programs = ScanForPrograms();
    DistributePrograms(programs);
    mpTabWidget->setCurrentIndex(0);
    onProgramUpdate(ui->sourceList);
    for (auto pList : { ui->sourceList, ui->sigprocList, ui->applicationList })
        pList->scrollToTop();
}

void MainWindow::on_actionCreate_Batch_File_triggered()
{
    QString dir = (FileUtils::InstallationDirectoryS() + "/../batch").c_str();
    QString file = QFileDialog::getSaveFileName(this, tr("Choose a name for the batch file"), dir, tr("BCI2000 Batch file (*.bat)"));
    if (file.isEmpty())
        return;

    std::string signalSource = "Start executable ";
    signalSource += GetSelectedProgram(mpSourceSelected).toUtf8().constData();
    signalSource += " --local ";
    signalSource += " --FileFormat=";
    signalSource += ui->fileFormatComboBox->currentData().toString().toLocal8Bit().constData();
    signalSource += ExtensionArgs("source").toUtf8().constData();

    std::string signalProcessing = "Start executable ";
    signalProcessing += GetSelectedProgram(mpSigProcSelected).toUtf8().constData();
    signalProcessing += " --local ";
    signalProcessing += ExtensionArgs("sigproc").toUtf8().constData();

    std::string application = "Start executable ";
    application += GetSelectedProgram(mpAppSelected).toUtf8().constData();
    application += " --local ";
    application += ExtensionArgs("app").toUtf8().constData();

    std::ostringstream scriptingCommands;
    WriteCommands("\r\n", scriptingCommands);

    std::map<std::string, std::string> vars;
    vars["SignalSourceCommand"] = signalSource;
    vars["SignalProcessingCommand"] = signalProcessing;
    vars["ApplicationCommand"] = application;
    vars["ScriptingCommands"] = scriptingCommands.str();

    std::string contents(bci::Resources::BatchFile_bat_in.data, bci::Resources::BatchFile_bat_in.length);
    contents = SubstituteVars(vars, contents);

    std::ofstream out(file.toLocal8Bit().constData(), std::ios::binary);
    out.write(contents.data(), contents.length());
}

void MainWindow::on_actionBCI2000_Help_triggered()
{
    ExecutableHelp().Display();
}

void MainWindow::SetSelectionLabel(QLabel* pLabel, const QString& value)
{
    if (value.isEmpty())
        ClearSelectionLabel(pLabel);
    else
        pLabel->setText(value);
}

void MainWindow::ClearSelectionLabel(QLabel* pLabel)
{
    if (pLabel == mpSourceSelected)
        pLabel->setText(cSourceDefault);
    else if (pLabel == mpSigProcSelected)
        pLabel->setText(cSigProcDefault);
    else if (pLabel == mpAppSelected)
        pLabel->setText(cAppDefault);
}

QString MainWindow::GetSelectedProgram(QLabel* pLabel)
{
    return pLabel->text();
}

void MainWindow::ClearExtensionSelections()
{
    auto items = ui->extensionsList->findItems(".*", Qt::MatchRegularExpression);
    for (auto pItem : std::as_const(items)) {
        for (int i = 0; i < pItem->childCount(); ++i) {
            auto pChild = pItem->child(i);
            pChild->setCheckState(0, Qt::Unchecked);
        }
    }
    for (auto& entry : mExtensionEnablerState) {
        entry.second.checked = false;
    }
    UpdateExtensionLabels();
}

void MainWindow::UpdateExtensionLabels()
{
    // sort by extension name first to be consistent with extension list view
    struct EnablersSorted : std::map<QString, QString> {
        QString serialize() const
        {
            QString s;
            for (const auto& entry : *this)
                s += entry.second + "\n";
            return s.trimmed();
        }
    };
    std::map<QString, EnablersSorted> enabledExtensions;
    for (const auto& entry : mExtensionEnablerState) {
        QString sortString = entry.second.extension + "\t" + entry.first;
        if (entry.second.checked) {
            enabledExtensions[entry.second.category][sortString] = entry.first;
        }
    }
    mpSourceExtensions->setText(enabledExtensions["source"].serialize());
    mpSigProcExtensions->setText(enabledExtensions["sigproc"].serialize());
    mpAppExtensions->setText(enabledExtensions["app"].serialize());
}

void MainWindow::SaveSettings() const
{
    Settings settings;
    settings.beginGroup("Config");
    settings.setValue("Geometry", this->saveGeometry());
    settings.setValue("VerticalSplitter1", mpVerticalSplitter1->saveState());
    settings.setValue("VerticalSplitter2", mpVerticalSplitter2->saveState());
    settings.setValue("HorizontalSplitter", mpHorizontalSplitter->saveState());
    settings.setValue("WorkingDirectory", QDir::currentPath());

    settings.beginWriteArray("BrokenModules");
    for (int i = 0; i < mBrokenModules.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("Name", mBrokenModules[i]);
    }
    settings.endArray();

    settings.beginWriteArray("HiddenModules");
    for (int i = 0; i < mHiddenModules.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("Name", mHiddenModules[i]);
    }
    settings.endArray();

    settings.beginWriteArray("Source Modules");
    for (int i = 0; i < ui->sourceList->count(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("Name", ui->sourceList->item(i)->text());
        settings.setValue("Selected", ui->sourceList->item(i)->isSelected());
    }
    settings.endArray();
    settings.setValue("SelectedSourceModule", mpSourceSelected->text());

    settings.beginWriteArray("Signal Processing Modules");
    for (int i = 0; i < ui->sigprocList->count(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("Name", ui->sigprocList->item(i)->text());
        settings.setValue("Selected", ui->sigprocList->item(i)->isSelected());
    }
    settings.endArray();
    settings.setValue("SelectedSigProcModule", mpSigProcSelected->text());

    settings.beginWriteArray("Application Modules");
    for (int i = 0; i < ui->applicationList->count(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("Name", ui->applicationList->item(i)->text());
        settings.setValue("Selected", ui->applicationList->item(i)->isSelected());
    }
    settings.endArray();
    settings.setValue("SelectedAppModule", mpAppSelected->text());

    settings.beginWriteArray("ExtensionEnablers");
    int i = 0;
    for (const auto& entry : mExtensionEnablerState)
    {
        settings.setArrayIndex(i);
        settings.setValue("Name", entry.first);
        settings.setValue("Checked", entry.second.checked);
        settings.setValue("Category", entry.second.category);
        settings.setValue("Extension", entry.second.extension);
        ++i;
    }
    settings.endArray();

    settings.beginWriteArray("ExtensionsCollapsed");
    i = 0;
    for (const auto& entry : mExtensionCollapsedState)
    {
        settings.setArrayIndex(i);
        settings.setValue("Name", entry.first);
        settings.setValue("Collapsed", entry.second);
        ++i;
    }
    settings.endArray();

    settings.setValue("ScriptingCommands", ui->scriptingCommandsEdit->toPlainText());

    settings.beginWriteArray("Parameter Files");
    for (int i = 0; i < mParameterFiles.count(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("Name", mParameterFiles.at(i));
    }
    settings.endArray();

    settings.setValue("Save Directory", ui->saveDirEdit->text());
    settings.setValue("Subject Name", ui->subjectNameEdit->text());
    settings.setValue("Subject Session", ui->subjectSessionEdit->text());
    settings.setValue("Data File Format", ui->fileFormatComboBox->currentData().toString());

    settings.endGroup();
}

void MainWindow::ReadSettings()
{
    Settings settings;
    settings.beginGroup("Config");
    this->restoreGeometry(settings.value("Geometry").toByteArray());
    mpVerticalSplitter1->restoreState(settings.value("VerticalSplitter1").toByteArray());
    mpVerticalSplitter2->restoreState(settings.value("VerticalSplitter2").toByteArray());
    mpHorizontalSplitter->restoreState(settings.value("HorizontalSplitter").toByteArray());
    QString wd = settings.value("WorkingDirectory").toString();
    if (wd.isEmpty())
        wd = FileUtils::InstallationDirectory().c_str();
    QDir::setCurrent(wd);

    mBrokenModules.clear();
    int size = settings.beginReadArray("BrokenModules");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        mBrokenModules.push_back(settings.value("Name").toString());
    }
    settings.endArray();

    mHiddenModules.clear();
    size = settings.beginReadArray("HiddenModules");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        mHiddenModules.push_back(settings.value("Name").toString());
    }
    settings.endArray();

    ui->sourceList->clear();
    size = settings.beginReadArray("Source Modules");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        CreateProgramItem(ui->sourceList, settings.value("Name").toString());
        ui->sourceList->item(i)->setSelected(settings.value("Selected").toBool());
    }
    settings.endArray();
    SetSelectionLabel(mpSourceSelected, settings.value("SelectedSourceModule").toString());

    auto selectedItems = ui->sourceList->selectedItems();
    if (!selectedItems.isEmpty())
        ui->sourceList->scrollToItem(selectedItems.front(), QAbstractItemView::PositionAtCenter);

    ui->sigprocList->clear();
    size = settings.beginReadArray("Signal Processing Modules");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        CreateProgramItem(ui->sigprocList, settings.value("Name").toString());
        ui->sigprocList->item(i)->setSelected(settings.value("Selected").toBool());
    }
    settings.endArray();
    selectedItems = ui->sigprocList->selectedItems();
    if (!selectedItems.isEmpty())
        ui->sigprocList->scrollToItem(selectedItems.front(), QAbstractItemView::PositionAtCenter);
    SetSelectionLabel(mpSigProcSelected, settings.value("SelectedSigProcModule").toString());

    ui->applicationList->clear();
    size = settings.beginReadArray("Application Modules");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        CreateProgramItem(ui->applicationList, settings.value("Name").toString());
        ui->applicationList->item(i)->setSelected(settings.value("Selected").toBool());
    }
    settings.endArray();
    selectedItems = ui->applicationList->selectedItems();
    if (!selectedItems.isEmpty())
        ui->applicationList->scrollToItem(selectedItems.front(), QAbstractItemView::PositionAtCenter);
    SetSelectionLabel(mpAppSelected, settings.value("SelectedAppModule").toString());

    size = settings.beginReadArray("ExtensionEnablers");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString name = settings.value("Name").toString();
        mExtensionEnablerState[name].checked = settings.value("Checked").toBool();
        mExtensionEnablerState[name].category = settings.value("Category").toString();
        mExtensionEnablerState[name].extension = settings.value("Extension").toString();
    }
    settings.endArray();

    size = settings.beginReadArray("ExtensionsCollapsed");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString name = settings.value("Name").toString();
        mExtensionCollapsedState[name] = settings.value("Collapsed").toBool();
    }
    settings.endArray();

    ui->scriptingCommandsEdit->setPlainText(settings.value("ScriptingCommands").toString());

    mParameterFiles.clear();
    ui->paramfilesList->clear();
    size = settings.beginReadArray("Parameter Files");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        mParameterFiles.append(settings.value("Name").toString());
        ui->paramfilesList->addItem(QFileInfo(mParameterFiles.last()).completeBaseName());
    }
    settings.endArray();

    ui->saveDirEdit->setText(settings.value("Save Directory").toString());
    ui->subjectNameEdit->setText(settings.value("Subject Name").toString());
    ui->subjectSessionEdit->setText(settings.value("Subject Session").toString());
    int idx = ui->fileFormatComboBox->findData(settings.value("Data File Format").toString());
    if (idx < 0) {
        idx = 0;
    }
    ui->fileFormatComboBox->setCurrentIndex(idx);

    settings.endGroup();
}

void MainWindow::on_resetSelectionBtn_clicked()
{
    for (auto pLabel : {mpSourceSelected, mpSigProcSelected, mpAppSelected})
        ClearSelectionLabel(pLabel);
    ClearExtensionSelections();
}

void MainWindow::on_actionUp_triggered()
{
    QListWidget *pListWidget = qobject_cast<QListWidget *>(this->focusWidget());
    if (pListWidget != NULL)
    {
        if (pListWidget->currentRow() > 0)
        {
            bool blocked = pListWidget->blockSignals(true);
            int targetRow = pListWidget->currentRow() - 1;
            QListWidgetItem *pItem = pListWidget->takeItem(pListWidget->currentRow());
            pListWidget->insertItem(targetRow, pItem);
            pListWidget->setCurrentRow(targetRow);
            pListWidget->blockSignals(blocked);
            onProgramUpdate(pListWidget);
        }
    }
}

void MainWindow::on_actionDown_triggered()
{
    QListWidget *pListWidget = qobject_cast<QListWidget *>(this->focusWidget());
    if (pListWidget != NULL)
    {
        if (pListWidget->currentRow() >= 0 && pListWidget->currentRow() < pListWidget->count() - 1)
        {
            bool blocked = pListWidget->blockSignals(true);
            int targetRow = pListWidget->currentRow() + 1;
            QListWidgetItem *pItem = pListWidget->takeItem(pListWidget->currentRow());
            pListWidget->insertItem(targetRow, pItem);
            pListWidget->setCurrentRow(targetRow);
            pListWidget->blockSignals(blocked);
            onProgramUpdate(pListWidget);
        }
    }
}

void MainWindow::on_actionTop_triggered()
{
    QListWidget* pListWidget = qobject_cast<QListWidget*>(this->focusWidget());
    if (pListWidget != nullptr)
    {
        if (pListWidget->currentRow() > 0)
        {
            bool blocked = pListWidget->blockSignals(true);
            int targetRow = 0;
            QListWidgetItem* pItem = pListWidget->takeItem(pListWidget->currentRow());
            pListWidget->insertItem(targetRow, pItem);
            pListWidget->setCurrentRow(targetRow);
            pListWidget->blockSignals(blocked);
            onProgramUpdate(pListWidget);
        }
    }
}

void MainWindow::on_actionBottom_triggered()
{
    QListWidget* pListWidget = qobject_cast<QListWidget*>(this->focusWidget());
    if (pListWidget != NULL)
    {
        if (pListWidget->currentRow() >= 0 && pListWidget->currentRow() < pListWidget->count() - 1)
        {
            bool blocked = pListWidget->blockSignals(true);
            int targetRow = pListWidget->count() - 1;
            QListWidgetItem* pItem = pListWidget->takeItem(pListWidget->currentRow());
            pListWidget->insertItem(targetRow, pItem);
            pListWidget->setCurrentRow(targetRow);
            pListWidget->blockSignals(blocked);
            onProgramUpdate(pListWidget);
        }
    }
}

void MainWindow::on_actionHide_triggered()
{
    QListWidget* pListWidget = qobject_cast<QListWidget*>(this->focusWidget());
    if (pListWidget != nullptr) {
        if (pListWidget->currentRow() >= 0 && pListWidget->currentRow() < pListWidget->count() - 1) {
            QListWidgetItem* pItem = pListWidget->item(pListWidget->currentRow());
            if (!mHiddenModules.contains(pItem->text()))
                mHiddenModules.push_back(pItem->text());
            delete pItem;
        }
    }
}

void MainWindow::on_addParmfileBtn_clicked()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select parameter file(s) to apply"),
                                                          QDir::currentPath(), tr("BCI2000 Parameter Files (*.prm)"));
    if (!fileNames.empty())
    {
        QDir::setCurrent(QFileInfo(fileNames.first()).absolutePath());
        for (QString file : fileNames)
        {
            ui->paramfilesList->addItem(QFileInfo(file).completeBaseName());
            mParameterFiles.append(file);
        }
    }
}

void MainWindow::on_paramfilesList_itemDoubleClicked(QListWidgetItem*)
{
    int idx = ui->paramfilesList->currentRow();
    if (idx >= 0)
    {
        std::string file = mParameterFiles[idx].toLocal8Bit().constData();
        std::string args = "--OpenParamView \"" + file + "\"";
        std::string executable = FindBCI2000Tool("BCI2000FileInfo");
        int exitCode = 0;
        if (!ProcessUtils::ExecuteAsynchronously(executable, args, exitCode)) {
            QMessageBox::critical(this, "BCI2000Launcher Error",
                QString("Could not launch BCI2000FileInfo executable: ") + SysError().Message());
        }
    }
}

void MainWindow::on_removeParmfileBtn_clicked()
{
    int idx = ui->paramfilesList->currentRow();
    if (idx >= 0)
    {
        QStringList::iterator i = mParameterFiles.begin() + idx;
        mParameterFiles.erase(i);
        delete ui->paramfilesList->takeItem(idx);
    }
}

void MainWindow::on_chooseDirBtn_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select top level directory to save sessions into"),
                                                    QDir::currentPath());
    if (!dir.isEmpty())
    {
        QDir::setCurrent(dir);
        ui->saveDirEdit->setText(dir);
        ui->saveDirEdit->setToolTip(dir);
    }
}

void MainWindow::on_subjectSessionEdit_editingFinished()
{
    QLineEdit *pSender = qobject_cast<QLineEdit *>(sender());
    if (pSender != NULL)
    {
        QString text = pSender->text();
        while (text.length() < 3)
            text = QString("0") + text;
        pSender->setText(text);
    }
}

void MainWindow::SyncToolTip()
{
    QLineEdit *pSender = qobject_cast<QLineEdit *>(sender());
    if (pSender != NULL)
        pSender->setToolTip(pSender->text());
}

QWidget* MainWindow::CreateHelpIcon(const QString& target) const
{
    QPixmap pixmap;
    pixmap.loadFromData(QByteArray(bci::Resources::QuestionMark_png.data, bci::Resources::QuestionMark_png.length));
    ClickableLabel* pLabel = new ClickableLabel;
    pLabel->setPixmap(pixmap.scaled(cHelpIconSize, cHelpIconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    pLabel->setAlignment(Qt::AlignBottom);
    pLabel->setProperty("HelpTarget", target);
    connect(pLabel, &ClickableLabel::clicked, this, &MainWindow::onHelpIconClicked);
    return pLabel;
}

void MainWindow::WriteCommands(const std::string& separator, std::ostream& os) const
{
    for (int i = 0; i < mParameterFiles.size(); ++i)
        os << "Load parameterfile " << EncodedString(mParameterFiles.at(i).toLocal8Bit().constData()) << separator;
    if (!ui->saveDirEdit->text().isEmpty())
        os << "Set parameter DataDirectory " << EncodeParamValue(ui->saveDirEdit->text().toLocal8Bit().constData()) << separator;
    if (!ui->subjectNameEdit->text().isEmpty())
        os << "Set parameter SubjectName " << EncodeParamValue(ui->subjectNameEdit->text().toLocal8Bit().constData()) << separator;
    if (!ui->subjectSessionEdit->text().isEmpty())
        os << "Set parameter SubjectSession " << EncodeParamValue(ui->subjectSessionEdit->text().toLocal8Bit().constData()) << separator;
    QStringList lines = ui->scriptingCommandsEdit->toPlainText().split('\n');
    for (const auto& line : std::as_const(lines)) {
        QString trimmedLine = line.trimmed();
        if (!trimmedLine.isEmpty() && trimmedLine[0] != '#')
            os << line.toUtf8().constData() << separator;
    }
    if (mParameterFiles.size() > 0)
        os << "SetConfig" << separator;
}

void MainWindow::on_subjectNameEdit_textChanged(QString)
{
    SyncToolTip();
}

void MainWindow::on_saveDirEdit_textChanged(QString)
{
    SyncToolTip();
}

void MainWindow::on_subjectSessionEdit_textChanged(QString)
{
    SyncToolTip();
}

void MainWindow::on_launchButton_clicked()
{
    bool expected = false;
    if (mLaunchButtonDisabled.compare_exchange_strong(expected, true))
        std::thread([this]{DoLaunch();}).detach();
}

void MainWindow::DoLaunch()
{
    struct LaunchButtonDisabler {
        LaunchButtonDisabler(std::atomic<bool>& flag)
            : mFlag(flag) { assert(mFlag); }
        ~LaunchButtonDisabler() { mFlag = false; }
        std::atomic<bool>& mFlag;

    } lauchButtonDisabler(mLaunchButtonDisabled);

    std::string progdir = FileUtils::InstallationDirectory();

    BCI2000Remote remote;
    remote.SetOperatorPath(progdir + "Operator");

    ClearLog();
    AppendLog(tr("Connecting to %1...").arg("Operator"));

    if (!remote.Connect()) {
        AppendLog(tr("Failed."));
        return;
    }

    remote.SetWindowVisible(true);
    remote.SetWindowTitle("");
    int result = 0;
    remote.Execute("Reset System", &result);
    remote.Execute("Change Directory \"" + progdir + "\"", &result);

    std::vector<std::string> modules;
    std::ostringstream module;

    module.str("");
    AppendLog(tr("Launching %1...").arg(GetSelectedProgram(mpSourceSelected)));
    module << progdir << GetSelectedProgram(mpSourceSelected).toLocal8Bit().constData();
    module << " --local";
    module << ExtensionArgs("source").toUtf8().constData();
    module << " --FileFormat=" << ui->fileFormatComboBox->currentData().toString().toLocal8Bit().constData();
    modules.push_back(module.str());

    module.str("");
    AppendLog(tr("Launching %1...").arg(GetSelectedProgram(mpSigProcSelected)));
    module << progdir << GetSelectedProgram(mpSigProcSelected).toLocal8Bit().constData();
    module << " --local";
    module << ExtensionArgs("sigproc").toUtf8().constData();
    modules.push_back(module.str());

    module.str("");
    AppendLog(tr("Launching %1...").arg(GetSelectedProgram(mpAppSelected)));
    module << progdir << GetSelectedProgram(mpAppSelected).toLocal8Bit().constData();
    module << " --local";
    module << ExtensionArgs("app").toUtf8().constData();
    modules.push_back(module.str());

    if (!remote.StartupModules(modules)) {
        AppendLog(tr("Failed: %1").arg(QString::fromUtf8(remote.Result().c_str())));
        return;
    }

    AppendLog(tr("Waiting for connection..."));
    if (!remote.Execute("Wait for Connected 5", &result)) {
        AppendLog(tr("Failed: %1").arg(QString::fromUtf8(remote.Result().c_str())));
        return;
    }

    std::ostringstream commands;
    WriteCommands("; ", commands);
    AppendLog(tr("Executing on connect script..."));
    if (!remote.Execute(commands.str(), &result)) {
        AppendLog(tr("Failed: %1").arg(QString::fromUtf8(remote.Result().c_str())));
        return;
    }

    AppendLog(tr("Finished."));
    remote.Disconnect(true);
}

void MainWindow::ClearLog()
{
    QMetaObject::invokeMethod(ui->statusEdit, "clear", Qt::AutoConnection);
}

void MainWindow::AppendLog(const QString& s)
{
    QMetaObject::invokeMethod(ui->statusEdit, "append", Qt::AutoConnection, Q_ARG(QString, s));
}

