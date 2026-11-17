////////////////////////////////////////////////////////////////////////////////
// $Id: BCI2000Viewer.cpp 9464 2026-07-18 20:45:55Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: The main window of the BCI2000Viewer tool.
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
#include "BCI2000Viewer.h"
#include "ui_BCI2000Viewer.h"

#include "QtMain.h"
#include "NotesWidget.h"
#include "AboutBox.h"
#include "AudioPlayer.h"
#include "AudioSink.h"
#include "ColorListChooser.h"
#include "ExecutableHelp.h"
#include "FileUtils.h"
#include "ProcessUtils.h"
#include "Settings.h"
#include "TimeValue.h"
#include "Streambuf.h"
#include "Resources.h"
#include "PhysicalUnit.h"
#include "ExtendedColorSelector.h"
#include "BCIStream.h"

#include <QtGui>

#include <iomanip>
#include <sstream>
#include <string>

Q_DECLARE_METATYPE(int64_t)

const double cAudioExtensionSamplingRate_Hz = 44100; // hardcoded in audio extension
const int cAudioExtensionDefaultBufferSize = 1024; // hardcoded in audio extension
const int cMaxWebcams = 32;

const RGBColor BCI2000Viewer::cAxisColor = RGBColor::White;
const RGBColor BCI2000Viewer::cChannelColorsDefault[] = {RGBColor::White, RGBColor::White,  RGBColor::White,
                                                         RGBColor::White, RGBColor::Yellow, ColorList::End};

const QColor BCI2000Viewer::cHeadingColor = QColor(0xffffb0);
const int BCI2000Viewer::cAutoDecimationMaxLinesPerPixel = 3;

BCI2000Viewer::ActionEntry BCI2000Viewer::sActions[] = {
    {&BCI2000Viewer::FileOpen, &BCI2000Viewer::FileOpen_Enabled, NULL, "actionOpen", NULL},
    {&BCI2000Viewer::FileClose, &BCI2000Viewer::FileClose_Enabled, NULL, "actionClose", NULL},
    {&BCI2000Viewer::FileOpenInfo, &BCI2000Viewer::FileOpenInfo_Enabled, NULL, "actionOpenInfo", NULL},
    {&BCI2000Viewer::FileExport, &BCI2000Viewer::FileExport_Enabled, NULL, "actionExport", NULL},
    {&BCI2000Viewer::FileQuit, &BCI2000Viewer::FileQuit_Enabled, NULL, "actionQuit", NULL},
    {&BCI2000Viewer::EditCopy, &BCI2000Viewer::EditCopy_Enabled, NULL, "actionCopy_to_Clipboard", NULL},
    {&BCI2000Viewer::HelpOpenHelp, NULL, NULL, "actionBCI2000_Help", NULL},
    {&BCI2000Viewer::HelpAbout, NULL, NULL, "actionAbout", NULL},

    {&BCI2000Viewer::ToggleNotes, &BCI2000Viewer::ToggleNotes_Enabled, &BCI2000Viewer::ToggleNotes_Checked,
    "actionToggle_Notes", nullptr},
    {&BCI2000Viewer::ToggleStatesChannels, &BCI2000Viewer::ToggleStatesChannels_Enabled, &BCI2000Viewer::ToggleStatesChannels_Checked,
    "actionToggle_StatesChannels", nullptr},
    {&BCI2000Viewer::ToggleFastScrolling, nullptr, &BCI2000Viewer::ToggleFastScrolling_Checked,
    "actionToggle_Fast_Scrolling", nullptr},
    {&BCI2000Viewer::ToggleAutoDecimation, nullptr, &BCI2000Viewer::ToggleAutoDecimation_Checked,
    "actionToggle_Auto_Decimation", nullptr},
    {&BCI2000Viewer::ColorThemeAuto, nullptr, &BCI2000Viewer::ColorThemeAuto_Checked, "actionAuto", "ColorThemeGroup"},
    {&BCI2000Viewer::ColorThemeLight, nullptr, &BCI2000Viewer::ColorThemeLight_Checked, "actionLight", "ColorThemeGroup"},
    {&BCI2000Viewer::ColorThemeDark, nullptr, &BCI2000Viewer::ColorThemeDark_Checked, "actionDark", "ColorThemeGroup"},

    {&BCI2000Viewer::EnlargeSignal, &BCI2000Viewer::ChangeResolution_Enabled, NULL, "actionEnlarge_Signal", NULL},
    {&BCI2000Viewer::ReduceSignal, &BCI2000Viewer::ChangeResolution_Enabled, NULL, "actionReduce_Signal", NULL},
    {&BCI2000Viewer::ResetSignalScale, &BCI2000Viewer::ResetSignalScale_Enabled, NULL, "actionReset", NULL},

    {&BCI2000Viewer::MoreChannels, &BCI2000Viewer::MoreChannels_Enabled, NULL, "actionMore_Channels", NULL},
    {&BCI2000Viewer::FewerChannels, &BCI2000Viewer::FewerChannels_Enabled, NULL, "actionFewer_Channels", NULL},
    {&BCI2000Viewer::ToggleColorSelector, &BCI2000Viewer::ToggleColorSelector_Enabled, &BCI2000Viewer::ToggleColorSelector_Checked,
     "actionChoose_Channel_Colors", NULL},
    {&BCI2000Viewer::ToggleGrid, &BCI2000Viewer::ToggleGrid_Enabled, &BCI2000Viewer::ToggleGrid_Checked,
     "actionShow_Grid", NULL},
    {&BCI2000Viewer::ToggleBaselines, &BCI2000Viewer::ToggleBaselines_Enabled, &BCI2000Viewer::ToggleBaselines_Checked,
     "actionShow_Baselines", NULL},
    {&BCI2000Viewer::ToggleUnit, &BCI2000Viewer::ToggleUnit_Enabled, &BCI2000Viewer::ToggleUnit_Checked,
     "actionShow_Unit", NULL},

    {&BCI2000Viewer::HPOff, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::HPOff_Checked, "actionHP_Off", "HP"},
    {&BCI2000Viewer::HPRemoveMean, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::HPRemoveMean_Checked, "actionHP_Remove_Mean", "HP"},
    {&BCI2000Viewer::HP1Hz, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::HP1Hz_Checked, "actionHP_1Hz", "HP"},
    {&BCI2000Viewer::HP5Hz, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::HP5Hz_Checked, "actionHP_5Hz", "HP"},
    {&BCI2000Viewer::HPCustom, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::HPCustom_Checked, "actionHP_Custom", "HP"},

    {&BCI2000Viewer::LPOff, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::LPOff_Checked, "actionLP_Off", "LP"},
    {&BCI2000Viewer::LP30Hz, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::LP30Hz_Checked, "actionLP_30Hz", "LP"},
    {&BCI2000Viewer::LP40Hz, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::LP40Hz_Checked, "actionLP_40Hz", "LP"},
    {&BCI2000Viewer::LP70Hz, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::LP70Hz_Checked, "actionLP_70Hz", "LP"},
    {&BCI2000Viewer::LPCustom, &BCI2000Viewer::Filter_Enabled, &BCI2000Viewer::LPCustom_Checked, "actionLP_Custom", "LP"},

    {&BCI2000Viewer::NotchOff, &BCI2000Viewer::Filter_Enabled, NULL, "actionNotch_Off", "Notch"},
    {&BCI2000Viewer::Notch50Hz, &BCI2000Viewer::Filter_Enabled, NULL, "actionNotch_50Hz", "Notch"},
    {&BCI2000Viewer::Notch60Hz, &BCI2000Viewer::Filter_Enabled, NULL, "actionNotch_60Hz", "Notch"},

    {&BCI2000Viewer::SampleZoomIn, &BCI2000Viewer::SampleZoomIn_Enabled, NULL, "actionZoom_In", NULL},
    {&BCI2000Viewer::SampleZoomOut, &BCI2000Viewer::SampleZoomOut_Enabled, NULL, "actionZoom_Out", NULL},
    {&BCI2000Viewer::ResetSampleZoom, &BCI2000Viewer::ResetSampleZoom_Enabled, NULL, "actionReset_Zoom", NULL},

    {&BCI2000Viewer::ToFirstSample, &BCI2000Viewer::GoBack_Enabled, NULL, "actionToBegin", NULL, "To Begin"},
    {&BCI2000Viewer::ToLastSample, &BCI2000Viewer::GoForward_Enabled, NULL, "actionToEnd", NULL, "To End"},
    {&BCI2000Viewer::ToPrevBlock, &BCI2000Viewer::GoBack_Enabled, NULL, "actionBlockRew", NULL, "Back one Block"},
    {&BCI2000Viewer::ToNextBlock, &BCI2000Viewer::GoForward_Enabled, NULL, "actionBlockFwd", NULL, "Forward one Block"},
    {&BCI2000Viewer::ToPrevPage, &BCI2000Viewer::GoBack_Enabled, NULL, "actionPageRew", NULL, "Back one Page"},
    {&BCI2000Viewer::ToNextPage, &BCI2000Viewer::GoForward_Enabled, NULL, "actionPageFwd", NULL, "Forward one Page"},

    {&BCI2000Viewer::ShowSelectedChannels, &BCI2000Viewer::ChannelsSelected, NULL, "actionShow_Channel_s", NULL},
    {&BCI2000Viewer::HideSelectedChannels, &BCI2000Viewer::ChannelsSelected, NULL, "actionHide_Channel_s", NULL},
    {&BCI2000Viewer::HelpOnState, &BCI2000Viewer::StateSelected, NULL, "actionState_Help", NULL},
};

//---------------------------------------------------------------------------

BCI2000Viewer::BCI2000Viewer(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::BCI2000Viewer), mSamplePos(0), 
      mNumSignalChannels(0), 
      mPositionEdited(false),
      mSamplingRate(0.0), mRemoveMean(false), 
      mpNotesWidget(nullptr), mpNotesState(nullptr),
      mpPlaybackControl(nullptr),
      mPrevReadSignalDuration(0),
      mpAudioSink(new AudioSink(bci::Rate::FromHertz(44100), 2, 441)),
      mpSecondaryViewer(nullptr),
      mIsPrimary(false), mIsSecondary(false)
{
    qRegisterMetaType<int64_t>("int64_t");

    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
    grabKeyboard();
    
    connect(QtApplication::instance(), &QtApplication::FileOpen, this, &BCI2000Viewer::onFileOpen);
    mpNotesWidget = new NotesWidget(this);
    mpNotesWidget->setVisible(false);
    ui->splitter->insertWidget(0, mpNotesWidget);
    connect(mpNotesWidget, &NotesWidget::userClickedNote, this, &BCI2000Viewer::SyncWithNote);

    ui->signalDisplay->SetBackgroundText("Drag and drop a BCI2000 data file\n"
                                         "into this window\nto display its contents.");
    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);
    ui->splitter->setCollapsible(2, false);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setStretchFactor(2, 0);
    ui->channelList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->menuWindow->setEnabled(false);

    SetupActions();
    ui->signalDisplay->Display()
        .SetValueUnitVisible(true)
        .SetAxisColor(cAxisColor)
        .SetChannelColors(cChannelColorsDefault)
        .SetFastScrolling(true)
        .SetAutoDecimation(cAutoDecimationMaxLinesPerPixel);
    mPlaybackTimer.setTimerType(Qt::PreciseTimer);
    mPlaybackTimer.setInterval(40); // 25 fps
    connect(&mPlaybackTimer, SIGNAL(timeout()), this, SLOT(PlaybackTimer()));

    QString filename;
    auto args = QApplication::arguments();
    for (int i = 1; i < args.size(); ++i)
    {
        if (args.at(i) == "--secondary")
            mIsSecondary = true;
        else if (args.at(i).startsWith("--"))
            /* ignore unknown option */;
        else
            filename = args.at(i);
    }
    if (mIsSecondary)
    {
        // disable close button on secondary viewer
        setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
        // make sure std streams are not redirected to bci streams
        BCIStream::ResetRedirection();
        // defer start of IO thread until events are being processed
        QTimer::singleShot(0, this, &BCI2000Viewer::OnIOThreadTimer);
    }
    if (!QDir::setCurrent((FileUtils::InstallationDirectory() + "/../../data").c_str()))
        QDir::setCurrent(ProcessUtils::InitialProcessWD().c_str());
    ReadSettings();
    show();
    DoFileOpen(filename);
}

BCI2000Viewer::~BCI2000Viewer()
{
    delete ui;
    delete mpColorSelector;
    delete mpSecondaryViewer;
    if (mSecondaryViewerThread.joinable())
        mSecondaryViewerThread.join();
}

void BCI2000Viewer::SetupActions()
{
    // setup context menu for signal display
    ui->signalDisplay->setContextMenuPolicy(Qt::CustomContextMenu);
    // create context menu for channel list
    ui->channelList->addAction(ui->actionShow_Channel_s);
    ui->channelList->addAction(ui->actionHide_Channel_s);
    QAction *pSeparator = new QAction(nullptr);
    pSeparator->setSeparator(true);
    ui->channelList->addAction(pSeparator);
    ui->channelList->addAction(ui->actionState_Help);
    ui->channelList->setContextMenuPolicy(Qt::ActionsContextMenu);

    // connect actions to slots and create action groups
    std::map<std::string, QActionGroup *> groups;
    for (size_t i = 0; i < sizeof(sActions) / sizeof(*sActions); ++i)
    {
        QPushButton* pButton = findChild<QPushButton*>(sActions[i].object);
        if (pButton)
        {
            connect(pButton, SIGNAL(clicked()), this, SLOT(ExecuteAction()));
        }
        QAction *pAction = findChild<QAction *>(sActions[i].object);
        if (!pButton && !pAction)
        {
            if (sActions[i].title)
            {
                pAction = new QAction(sActions[i].title, this);
                pAction->setObjectName(sActions[i].object);
            }
        }
        if (pAction)
        {
            connect(pAction, SIGNAL(triggered()), this, SLOT(ExecuteAction()));
            if (sActions[i].group != NULL)
            {
                pAction->setCheckable(true);
                std::string g = sActions[i].group;
                if (groups[g] == NULL)
                {
                    groups[g] = new QActionGroup(this);
                    pAction->setChecked(true);
                }
                groups[g]->addAction(pAction);
            }
            else if (sActions[i].checked != NULL)
                pAction->setCheckable(true);
        }
    }
    PopulateToolbars();
    LoadActionIcons();
    ApplyColorTheme();
}

void BCI2000Viewer::ExecuteAction()
{
    QObject *pSender = sender();
    for (size_t i = 0; i < sizeof(sActions) / sizeof(*sActions); ++i)
        if (pSender->objectName() == sActions[i].object)
            (this->*sActions[i].action)();
    UpdateActions();
}

void BCI2000Viewer::UpdateActions()
{
    for (size_t i = 0; i < sizeof(sActions) / sizeof(*sActions); ++i)
    {
        QAction *pAction = findChild<QAction *>(sActions[i].object);
        if (pAction)
        {
            if (sActions[i].enabled)
                pAction->setEnabled((this->*sActions[i].enabled)());
            if (sActions[i].checked)
                pAction->setChecked((this->*sActions[i].checked)());
        }
        QPushButton *pButton = findChild<QPushButton *>(sActions[i].object);
        if (pButton)
        {
            if (sActions[i].enabled)
                pButton->setEnabled((this->*sActions[i].enabled)());
        }
    }

    mpColorThemeButton->setEnabled(mFile.IsOpen());
    mpHPButton->setEnabled(mFile.IsOpen());
    mpLPButton->setEnabled(mFile.IsOpen());
    mpNotchButton->setEnabled(mFile.IsOpen());
}

void BCI2000Viewer::closeEvent(QCloseEvent *pEvent)
{
    SaveSettings();
    CloseNotes();
    DestroyAVObjects();
    DestroySecondaryViewer();
    pEvent->accept();
}

void BCI2000Viewer::keyPressEvent(QKeyEvent *iopEvent)
{
    static int acc = 0;
    int wipe_acc = 1;
    bool shift = (iopEvent->modifiers() & Qt::ShiftModifier) || (iopEvent->modifiers() & Qt::ControlModifier) ||
                 (iopEvent->modifiers() & Qt::AltModifier);
    switch (iopEvent->key())
    {
    case Qt::Key_Up:
        if (ChannelUp_Enabled())
        {
            if (shift)
                ChannelPagePrev();
            else
                ChannelUp();
        }
        break;
    case Qt::Key_Down:
        if (ChannelDown_Enabled())
        {
            if (shift)
                ChannelPageNext();
            else
                ChannelDown();
        }
        break;

    case Qt::Key_PageUp:
    case Qt::Key_B:
        if (ChannelUp_Enabled())
            ChannelPagePrev();
        break;
    case Qt::Key_PageDown:
    case Qt::Key_Space:
        if (ChannelDown_Enabled())
            ChannelPageNext();
        break;

    case Qt::Key_Right:
        if (GoForward_Enabled())
        {
            if (shift)
                ToNextPage();
            else
                ToNextBlock();
        }
        break;
    case Qt::Key_Left:
        if (GoBack_Enabled())
        {
            if (shift)
                ToPrevPage();
            else
                ToPrevBlock();
        }
        break;
    case Qt::Key_Comma:
        if (FewerChannels_Enabled())
            FewerChannels();
        break;
    case Qt::Key_Period:
        if (MoreChannels_Enabled())
            MoreChannels();
        break;
    case Qt::Key_Minus:
        if (ChangeResolution_Enabled())
            ReduceSignal();
        break;
    case Qt::Key_Plus:
        if (ChangeResolution_Enabled())
            EnlargeSignal();
        break;
    case Qt::Key_Home:
        if (ChannelUp_Enabled())
            ChannelPageFirst();
        break;
    case Qt::Key_End:
        if (ChannelDown_Enabled())
            ChannelPageLast();
        break;
    case Qt::Key_Escape:
        ClearChannelSelection();
        break;
    case Qt::Key_Enter:
    case Qt::Key_G:
        ChannelPageFirst();
        for (int i = 1; i < acc; ++i)
            ChannelDown();
        break;
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        acc = acc * 10 + (iopEvent->key() - Qt::Key_0);
        wipe_acc = 0;
        break;
    }
    if (wipe_acc)
        acc = 0;
}

void BCI2000Viewer::mousePressEvent(QMouseEvent* iopEvent)
{
    if (iopEvent->buttons() == Qt::LeftButton) {
        QPointF where = iopEvent->localPos();
        for (QWidget* pWidget = ui->signalDisplay; pWidget != this; pWidget = pWidget->parentWidget())
            where -= pWidget->pos();
        int sample = ui->signalDisplay->Display().PosToSample(where.x());
        int ch = ui->signalDisplay->Display().PosToChannel(where.y());
        if (ch < 0 || ch >= mChannelsSelected.size() || sample < 0 || sample >= ui->signalDisplay->Display().NumSamples()) {
            ClearChannelSelection();
        }
        else {
            int modifiers = iopEvent->modifiers();
            if (modifiers & Qt::ShiftModifier) {
                ExtendChannelSelection(ch);
            }
            else if (modifiers & Qt::CTRL) {
                if (ToggleChannelSelected(ch))
                    mSelectionAnchor = ch;
            }
            else {
                ClearChannelSelection();
                SetChannelSelected(ch, true);
                mSelectionAnchor = ch;
            }
        }
        mDMouseY = 0;
        mMouseY = where.y();
    }
    QWidget::mousePressEvent(iopEvent);
}

void BCI2000Viewer::mouseMoveEvent(QMouseEvent* iopEvent)
{
    if (iopEvent->buttons() == Qt::LeftButton) {
        QPointF where = iopEvent->localPos();
        for (QWidget* pWidget = ui->signalDisplay; pWidget != this; pWidget = pWidget->parentWidget())
            where -= pWidget->pos();
        mDMouseY = where.y() - mMouseY;
        // If mouse is moving away from the selection anchor, keep selecting channels.
        // Otherwise, keep deselecting channels.
        auto channels = YRangeToChannels(mMouseY, where.y());
        for (int ch = channels.first; ch <= channels.second; ++ch) {
            if (ch > 0 && ch <= mChannelsSelected.size() && IsChannelVisible(ch)) {
                int relLoc = ch - mSelectionAnchor;
                bool doSelect = (mDMouseY * relLoc >= 0);
                SetChannelSelected(ch, doSelect);
            }
        }
        mMouseY = where.y();
    }
    QWidget::mouseMoveEvent(iopEvent);
}

void BCI2000Viewer::wheelEvent(QWheelEvent *iopEvent)
{
    double deltaY = iopEvent->angleDelta().y();
    if (deltaY > 0) {
        if (ChannelUp_Enabled())
        {
            if (iopEvent->modifiers() & Qt::ShiftModifier)
                ChannelPagePrev();
            else
                ChannelUp();
        }
        iopEvent->accept();
    }
    else if (deltaY < 0) {
        if (ChannelDown_Enabled())
        {
            if (iopEvent->modifiers() & Qt::ShiftModifier)
                ChannelPageNext();
            else
                ChannelDown();
        }
        iopEvent->accept();
    }
    else {
        iopEvent->ignore();
    }
}

void BCI2000Viewer::dragEnterEvent(QDragEnterEvent *iopEvent)
{
    if (mIsSecondary)
        return;
    QList<QUrl> urlList = iopEvent->mimeData()->urls();
    if (urlList.size() == 1)
    {
        QString file = urlList[0].toLocalFile();
        if (QFileInfo(file).isFile() && BCI2000FileReader(file.toLocal8Bit().constData()).IsOpen())
            iopEvent->acceptProposedAction();
    }
}

void BCI2000Viewer::dropEvent(QDropEvent *iopEvent)
{
    if (mIsSecondary)
        return;
    QList<QUrl> urlList = iopEvent->mimeData()->urls();
    if (urlList.size() == 1)
        DoFileOpen(urlList[0].toLocalFile());
    iopEvent->acceptProposedAction();
}

void BCI2000Viewer::changeEvent(QEvent* iopEvent)
{
    if (iopEvent->type() == QEvent::PaletteChange) {
        LoadActionIcons();
        ApplyColorTheme();
    }
    QWidget::changeEvent(iopEvent);
}

bool BCI2000Viewer::event(QEvent* iopEvent)
{
    if (iopEvent->type() == QEvent::ToolTip) {
        QHelpEvent* pHelpEvent = static_cast<QHelpEvent*>(iopEvent);
        if (onSignalDisplayToolTip(pHelpEvent->globalPos()))
            return true;
    }
    return QWidget::event(iopEvent);
}

void BCI2000Viewer::onFileOpen(const QString& filename)
{
    if (mIsSecondary)
        return;
    if (!filename.isEmpty())
    {
        QDir::setCurrent(QFileInfo(filename).canonicalPath());
        DoFileOpen(filename);
    }
}

void BCI2000Viewer::OnEditPositionTextEdited(QString)
{
    mPositionEdited = true;
}

void BCI2000Viewer::OnEditPositionEditingFinished()
{
    if (mPositionEdited)
    {
        auto pEditPosition = findChild<QLineEdit*>("editPosition");
        TimeValue t;
        std::istringstream iss(pEditPosition->text().toLocal8Bit().constData());
        if (iss >> t)
            SetSamplePos(t * mFile.SamplingRate() - ui->signalDisplay->Display().NumSamples() / 2);
    }
    this->grabKeyboard();
}

void BCI2000Viewer::OnEditPositionReturnPressed()
{
    OnEditPositionEditingFinished();
}

void BCI2000Viewer::OnEditPositionSelectionChanged()
{
    mPositionEdited = false;
    this->releaseKeyboard();
}

void BCI2000Viewer::on_channelList_itemChanged(QListWidgetItem *)
{
    int begin = ui->channelList->count() - mNumSignalChannels;
    Assert(begin >= 0);
    for (int row = begin; row < ui->channelList->count(); ++row) {
        bool checked = ui->channelList->item(row)->checkState() == Qt::Checked;
        ui->signalDisplay->Display().SetChannelVisible(row - begin, checked);
    }
    UpdateChannelLabels();
    SetSamplePos(mSamplePos);
    UpdateVerticalScroller();
}

void BCI2000Viewer::on_channelList_itemSelectionChanged()
{
    int begin = ui->channelList->count() - mChannelsSelected.size();
    Assert(begin >= 0);
    for (int row = begin; row < ui->channelList->count(); ++row) {
        bool selected = ui->channelList->item(row)->isSelected();
        mChannelsSelected[row - begin] = selected;
    }
    ui->signalDisplay->Display().SetChannelSelected(mChannelsSelected);
    UpdateActions();
}

void BCI2000Viewer::on_channelList_currentRowChanged(int inCurrentRow)
{
    UpdateActions();
}

void BCI2000Viewer::on_verticalScrollBar_valueChanged(int value)
{
    ui->signalDisplay->Display().SetTopGroup(ui->verticalScrollBar->sliderPosition());
}

bool BCI2000Viewer::onSignalDisplayToolTip(const QPoint& global)
{
    QPoint where = ui->signalDisplay->mapFromGlobal(global);
    const SignalDisplay& display = ui->signalDisplay->Display();
    int sample = display.PosToSample(where.x());
    if (sample < 0)
        return false;
    if (sample >= display.NumSamples())
        return false;
    int ch = display.PosToMarkerChannel(where.y());
    if (ch >= 0)
        ch += display.NumChannels() - display.NumMarkerChannels();
    else
        ch = display.PosToChannel(where.y());
    if (ch < 0)
        return false;

    double value = display.Data(ch, sample);
    QString valStr;
    if (ch < display.ValueUnits().size()) {
        std::string s = PhysicalUnit().SetGainWithSymbol(display.ValueUnits()[ch]).RawToPhysical(value).Utf8();
        valStr = QString::fromUtf8(s.c_str());
    }
    else {
        valStr = QString::number(value);
    }
    QString text = QString("%1 = %2").arg(display.ChannelLabels()[ch].Text().c_str()).arg(valStr);
    QRect rect(where - QPoint(2, 2), where + QPoint(2, 2)); // will hide tooltip if pointer is moved
    QToolTip::showText(global, text, ui->signalDisplay, rect);
    return true;
}

void BCI2000Viewer::on_signalDisplay_customContextMenuRequested(QPoint inPos)
{
    QWidget *pWidget = dynamic_cast<QWidget *>(sender());
    if (pWidget != NULL)
        ui->menuView->popup(pWidget->mapToGlobal(inPos));
}

// User actions
//---------------------------------------------------------------------------
// Standard actions
void BCI2000Viewer::FileOpen()
{
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open Data File"), QDir::currentPath(), tr("BCI2000 Data Files (*.dat)"));
    if (!filename.isEmpty())
    {
        QDir::setCurrent(QFileInfo(filename).canonicalPath());
        DoFileOpen(filename);
    }
}
bool BCI2000Viewer::FileOpen_Enabled() const
{
    return !mIsSecondary;
}

void BCI2000Viewer::FileClose()
{
    DoFileOpen("");
}
bool BCI2000Viewer::FileClose_Enabled() const
{
    return mFile.IsOpen() && !mIsSecondary;
}

static std::string ConstructFileInfoPath()
{
    std::string dir = FileUtils::InstallationDirectory();
    dir = FileUtils::ParentDirectory(dir);
    dir += "/BCI2000FileInfo/";
    std::string executable = dir + "BCI2000FileInfo";
#if _WIN32
    executable += ".exe";
#endif
#if __APPLE__ && USE_MACOS_APPLICATION_BUNDLES
    executable += ".app/Contents/MacOS/BCI2000FileInfo";
#endif
    return executable;
}

void BCI2000Viewer::FileOpenInfo()
{
    std::string executable = ConstructFileInfoPath();
    int ignored = 0;
    ProcessUtils::ExecuteAsynchronously(executable, mFileName, ignored);
}

bool BCI2000Viewer::FileOpenInfo_Enabled() const
{
    std::string executable = ConstructFileInfoPath();
    return FileUtils::IsFile(executable) && mFile.IsOpen();
}

void BCI2000Viewer::FileExport()
{
    QString file = QString::fromLocal8Bit(
        (FileUtils::ExtractDirectory(mFileName) + FileUtils::ExtractBase(mFileName) + ".ascii").c_str());
    QString filename =
        QFileDialog::getSaveFileName(this, tr("Save currently displayed data as ..."), file,
                                     "Tab-separated ASCII file (*.ascii);;Octave-style ASCII file (*.oct)");
    if (!filename.isEmpty())
    {
        std::string file = filename.toLocal8Bit().constData();
        std::string ext = FileUtils::ExtractExtension(file);
        std::ofstream output(file);
        if (!output.is_open())
        {
            bcierr << "Could not open\n" << file << "\nfor writing.";
            return;
        }
        if (ext == ".ascii")
        {
            const LabelList &labels = ui->signalDisplay->Display().ChannelLabels();
            for (int ch = 0; ch < labels.Size(); ++ch)
            {
                output << labels[ch].Text();
                if (ch < labels.Size() - 1)
                    output << "\t";
                else
                    output << "\n";
            }
            GenericSignal signal = ConstructDisplaySignal(0, mFile.NumSamples());
            output << std::scientific << std::setprecision(std::numeric_limits<double>::max_digits10);
            for (int el = 0; output && el < signal.Elements(); ++el)
            {
                for (int ch = 0; ch < signal.Channels(); ++ch)
                {
                    output << signal(ch, el);
                    if (ch < signal.Channels() - 1)
                        output << "\t";
                    else
                        output << "\n";
                }
            }
        }
        else if (ext == ".oct")
        {
            output << std::setprecision(std::numeric_limits<double>::max_digits10);
            GenericSignal signal = ConstructDisplaySignal(0, mFile.NumSamples());
            const LabelList &labels = ui->signalDisplay->Display().ChannelLabels();
            int states = ui->signalDisplay->Display().NumMarkerChannels(), channels = signal.Channels() - states;
            if (channels > 0)
            {
                output << "# name: signal\n# type: matrix\n# rows: " << signal.Elements() << "\n# columns: " << channels
                       << "\n";
                for (int el = 0; el < signal.Elements(); ++el)
                {
                    for (int ch = 0; ch < channels; ++ch)
                        output << ' ' << signal(ch, el);
                    output << '\n';
                }
            }
            for (int ch = channels; ch < signal.Channels(); ++ch)
            {
                output << "# name: " << labels[ch].Text() << "\n# type: matrix\n# rows: " << signal.Elements()
                       << "\n# columns: 1\n";
                for (int el = 0; el < signal.Elements(); ++el)
                    output << ' ' << signal(ch, el) << '\n';
            }
        }
        else
        {
            bcierr << "Unknown file format: " << ext;
            return;
        }
        if (!(output << std::flush))
            bcierr << "Error when writing\n" << file;
    }
}

bool BCI2000Viewer::FileExport_Enabled() const
{
    return mFile.IsOpen();
}

void BCI2000Viewer::FileQuit()
{
    close();
}

bool BCI2000Viewer::FileQuit_Enabled() const
{
    return !mIsSecondary;
}

void BCI2000Viewer::EditCopy()
{
    QImage image(ui->signalDisplay->width(), ui->signalDisplay->height(), QImage::Format_RGB32);
    ;
    ui->signalDisplay->render(&image);
    QApplication::clipboard()->setImage(image);
}

bool BCI2000Viewer::EditCopy_Enabled() const
{
    return mFile.IsOpen();
}

void BCI2000Viewer::HelpOpenHelp()
{
    ExecutableHelp().Display();
}

void BCI2000Viewer::HelpAbout()
{
    AboutBox().SetApplicationName(QApplication::applicationName().toLocal8Bit().constData()).Display();
}

// Temporal movement
void BCI2000Viewer::ToFirstSample()
{
    SetSamplePos(0);
}

void BCI2000Viewer::ToLastSample()
{
    SetSamplePos(LastSample());
}

void BCI2000Viewer::ToNextBlock()
{
    ScrollSamplePosBy(mFile.SignalProperties().Elements());
}

void BCI2000Viewer::ToPrevBlock()
{
    ScrollSamplePosBy(-mFile.SignalProperties().Elements());
}

void BCI2000Viewer::ToPrevPage()
{
    SetSamplePos(std::max<int64_t>(0LL, mSamplePos - ui->signalDisplay->Display().NumSamples()));
}

void BCI2000Viewer::ToNextPage()
{
    SetSamplePos(std::min(mFile.NumSamples() - ui->signalDisplay->Display().NumSamples(),
                          mSamplePos + ui->signalDisplay->Display().NumSamples()));
}

bool BCI2000Viewer::GoBack_Enabled() const
{
    return mSamplePos > 0;
}
bool BCI2000Viewer::GoForward_Enabled() const
{
    return mSamplePos < mFile.NumSamples() - ui->signalDisplay->Display().NumSamples() - 1;
}

// Temporal resolution
void BCI2000Viewer::SampleZoomIn()
{
    int prevNumSamples = NumDisplaySamples(), newNumSamples = prevNumSamples / 2;
    SetNumDisplaySamples(newNumSamples);
    SetSamplePos(mSamplePos + (prevNumSamples - newNumSamples) / 2);
}

bool BCI2000Viewer::SampleZoomIn_Enabled() const
{
    return mFile.IsOpen() && NumDisplaySamples() / 2 >= mFile.SignalProperties().Elements();
}

void BCI2000Viewer::SampleZoomOut()
{
    int prevNumSamples = NumDisplaySamples(), newNumSamples = prevNumSamples * 2;
    if (newNumSamples > mFile.NumSamples())
        newNumSamples = mFile.NumSamples();
    SetNumDisplaySamples(newNumSamples);
    SetSamplePos(mSamplePos + (prevNumSamples - newNumSamples) / 2);
}

bool BCI2000Viewer::SampleZoomOut_Enabled() const
{
    return NumDisplaySamples() < mFile.NumSamples();
}

void BCI2000Viewer::ResetSampleZoom()
{
    int prevNumSamples = NumDisplaySamples();
    SetNumDisplaySamples(mNumSamplesDefault);
    SetSamplePos(mSamplePos + (prevNumSamples - mNumSamplesDefault) / 2);
}

bool BCI2000Viewer::ResetSampleZoom_Enabled() const
{
    return mFile.IsOpen();
}

// Number of displayed channels
void BCI2000Viewer::FewerChannels()
{
    int newNumChannels = ui->signalDisplay->Display().DisplayGroups() / 2;
    if (newNumChannels < 1)
        newNumChannels = 1;
    ui->signalDisplay->Display().SetDisplayGroups(newNumChannels).SetTopGroup(ui->verticalScrollBar->sliderPosition());
    UpdateVerticalScroller();
}

bool BCI2000Viewer::FewerChannels_Enabled() const
{
    return ui->signalDisplay->Display().DisplayGroups() > 1;
}

void BCI2000Viewer::MoreChannels()
{
    int newNumChannels = ui->signalDisplay->Display().DisplayGroups() * 2;
    newNumChannels = std::min(newNumChannels, mNumSignalChannels);
    ui->signalDisplay->Display().SetDisplayGroups(newNumChannels).SetTopGroup(ui->verticalScrollBar->sliderPosition());
    UpdateVerticalScroller();
}

bool BCI2000Viewer::MoreChannels_Enabled() const
{
    return ui->signalDisplay->Display().DisplayGroups() < mNumSignalChannels;
}

// Channel scrolling
void BCI2000Viewer::ChannelUp()
{
    ui->verticalScrollBar->setSliderPosition(ui->verticalScrollBar->sliderPosition() - 1);
}

void BCI2000Viewer::ChannelDown()
{
    ui->verticalScrollBar->setSliderPosition(ui->verticalScrollBar->sliderPosition() + 1);
}

void BCI2000Viewer::ChannelPageNext()
{
    QScrollBar *s = ui->verticalScrollBar;
    s->setSliderPosition(std::min(s->maximum(), s->sliderPosition() + s->pageStep()));
}

void BCI2000Viewer::ChannelPagePrev()
{
    QScrollBar *s = ui->verticalScrollBar;
    s->setSliderPosition(std::max(s->minimum(), s->sliderPosition() - s->pageStep()));
}

void BCI2000Viewer::ChannelPageFirst()
{
    ui->verticalScrollBar->setSliderPosition(ui->verticalScrollBar->minimum());
}

void BCI2000Viewer::ChannelPageLast()
{
    ui->verticalScrollBar->setSliderPosition(ui->verticalScrollBar->maximum());
}

bool BCI2000Viewer::ChannelUp_Enabled()
{
    QScrollBar *s = ui->verticalScrollBar;
    return s->isEnabled() && s->sliderPosition() > s->minimum();
}

bool BCI2000Viewer::ChannelDown_Enabled()
{
    QScrollBar *s = ui->verticalScrollBar;
    return s->isEnabled() && s->sliderPosition() < s->maximum();
}

// Signal resolution
void BCI2000Viewer::RescaleSignal(char op, double maxValue, double minValue)
{
    auto selected = ExtendedChannelSelection();

    SignalDisplay& display = ui->signalDisplay->Display();
    auto minValues = display.MinValues(), maxValues = display.MaxValues();
    minValues.resize(selected.size(), display.MinValue());
    maxValues.resize(selected.size(), display.MaxValue());

    for (int i = 0; i < selected.size(); ++i) {
        if (selected[i]) {
            switch (op) {
                case '*':
                    minValues[i] *= minValue;
                    maxValues[i] *= maxValue;
                    break;
                case '/':
                    minValues[i] /= minValue;
                    maxValues[i] /= maxValue;
                    break;
                case '=':
                    minValues[i] = minValue;
                    maxValues[i] = maxValue;
                    break;
            }
        }
    }
    display.SetMinValues(minValues);
    display.SetMaxValues(maxValues);
}

void BCI2000Viewer::EnlargeSignal()
{
    RescaleSignal('/', 2, 2);
}

void BCI2000Viewer::ReduceSignal()
{
    RescaleSignal('*', 2, 2);
}

bool BCI2000Viewer::ChangeResolution_Enabled() const
{
    return ui->signalDisplay->Display().NumSamples() > 0;
}

void BCI2000Viewer::ResetSignalScale()
{
    RescaleSignal('=', mMinValueDefault, mMaxValueDefault);
}

bool BCI2000Viewer::ResetSignalScale_Enabled() const
{
    return ui->signalDisplay->Display().NumSamples() > 0;
}

// Display attributes
void BCI2000Viewer::ToggleColorSelector()
{
    if (!mpColorSelector) {
        mpColorSelector = new ExtendedColorSelector;
        mpColorSelector->setWindowTitle("Choose Channel Colors...");
        connect(mpColorSelector, &ExtendedColorSelector::colorApplied, this, &BCI2000Viewer::ApplyColorSelection);
        connect(mpColorSelector, &ExtendedColorSelector::colorPatternApplied, this, &BCI2000Viewer::ApplyColorPattern);
        connect(mpColorSelector, &ExtendedColorSelector::resetClicked, this, &BCI2000Viewer::ApplyColorReset);
        connect(mpColorSelector, &ExtendedColorSelector::closed, this, &BCI2000Viewer::ColorSelectorClosed);
    }
    mpColorSelector->setVisible(!mpColorSelector->isVisible());
    if (mpColorSelector->isVisible())
    {
        mpColorSelector->setApplyColorButtonEnabled(!ui->channelList->selectedItems().empty());
        mpColorSelector->activateWindow();
        // make a list of unique channel colors to populate the custom colors array
        ColorList channelColors = ui->signalDisplay->Display().ChannelColors();
        std::vector<QColor> uniqueColors;
        for (auto color : channelColors) {
            QColor c = color.ToQRgb();
            if (std::find(uniqueColors.begin(), uniqueColors.end(), c) == uniqueColors.end())
                uniqueColors.push_back(c);
        }
        mpColorSelector->setCustomColors(uniqueColors);
        mOriginalColors = channelColors;
        this->releaseKeyboard();
    }
    else
    {
        mOriginalColors.clear();
        this->grabKeyboard();
    }
}

bool BCI2000Viewer::ToggleColorSelector_Enabled() const
{
    return mFile.IsOpen();
}

bool BCI2000Viewer::ToggleColorSelector_Checked() const
{
    return mpColorSelector && mpColorSelector->isVisible();
}

void BCI2000Viewer::ColorSelectorClosed()
{
    ui->actionChoose_Channel_Colors->setChecked(false);
}

void BCI2000Viewer::ApplyColorSelection(const QColor& color)
{
    ColorList channelColors = ui->signalDisplay->Display().ChannelColors();
    int oldSize = channelColors.size();
    for (int i = oldSize; i < mChannelsSelected.size(); ++i)
        channelColors.push_back(channelColors[i - oldSize]);
    for (int i = 0; i < mChannelsSelected.size(); ++i)
        if (mChannelsSelected[i])
            channelColors[i] = RGBColor::FromQRgb(color.rgba());
    ui->signalDisplay->Display().SetChannelColors(channelColors);
}

void BCI2000Viewer::ApplyColorPattern(const QString& countStr)
{
    int count = countStr.toInt();
    if (count > 1) {
        ColorList channelColors = ui->signalDisplay->Display().ChannelColors();
        int oldSize = channelColors.size();
        for (int i = oldSize; i < count; ++i)
            channelColors.push_back(channelColors[i - oldSize]);
        channelColors.resize(count);
        ui->signalDisplay->Display().SetChannelColors(channelColors);
    }
}

void BCI2000Viewer::ApplyColorReset()
{
    ui->signalDisplay->Display().SetChannelColors(mOriginalColors);
}

void BCI2000Viewer::ToggleFastScrolling()
{
    ui->signalDisplay->Display().SetFastScrolling(!ui->signalDisplay->Display().FastScrolling());
}

bool BCI2000Viewer::ToggleFastScrolling_Checked() const
{
    return ui->signalDisplay->Display().FastScrolling();
}

void BCI2000Viewer::ToggleAutoDecimation()
{
    int maxLpp = ui->signalDisplay->Display().AutoDecimation();
    if (maxLpp)
        maxLpp = 0;
    else
        maxLpp = cAutoDecimationMaxLinesPerPixel;
    ui->signalDisplay->Display().SetAutoDecimation(maxLpp);
}

bool BCI2000Viewer::ToggleAutoDecimation_Checked() const
{
    return ui->signalDisplay->Display().AutoDecimation();
}

void BCI2000Viewer::ColorThemeAuto()
{
    mColorTheme = auto_;
    ApplyColorTheme();
}

bool BCI2000Viewer::ColorThemeAuto_Checked() const
{
    return mColorTheme == auto_;
}

void BCI2000Viewer::ColorThemeLight()
{
    mColorTheme = light_;
    ApplyColorTheme();
}

bool BCI2000Viewer::ColorThemeLight_Checked() const
{
    return mColorTheme == light_;
}

void BCI2000Viewer::ColorThemeDark()
{
    mColorTheme = dark_;
    ApplyColorTheme();
}

bool BCI2000Viewer::ColorThemeDark_Checked() const
{
    return mColorTheme == dark_;
}

namespace {
    bool IsPaletteDark(const QPalette& palette)
    {
        return palette.text().color().lightness() > palette.window().color().lightness();
    }

}

void BCI2000Viewer::ApplyColorTheme()
{
    bool lightMode = (mColorTheme == light_);
    if (mColorTheme == auto_)
        lightMode = !IsPaletteDark(this->palette());
    ui->signalDisplay->Display().SetLightMode(lightMode);
}

void BCI2000Viewer::ToggleGrid()
{
    ui->signalDisplay->Display().SetGridVisible(!ui->signalDisplay->Display().GridVisible());
}

bool BCI2000Viewer::ToggleGrid_Checked() const
{
    return ui->signalDisplay->Display().GridVisible();
}

bool BCI2000Viewer::ToggleGrid_Enabled() const
{
    return mNumSignalChannels > 0;
}

void BCI2000Viewer::ToggleBaselines()
{
    ui->signalDisplay->Display().SetBaselinesVisible(!ui->signalDisplay->Display().BaselinesVisible());
}

bool BCI2000Viewer::ToggleBaselines_Checked() const
{
    return ui->signalDisplay->Display().BaselinesVisible();
}

bool BCI2000Viewer::ToggleBaselines_Enabled() const
{
    return mNumSignalChannels > 0;
}

void BCI2000Viewer::ToggleUnit()
{
    ui->signalDisplay->Display().SetValueUnitVisible(!ui->signalDisplay->Display().ValueUnitVisible());
}

bool BCI2000Viewer::ToggleUnit_Checked() const
{
    return ui->signalDisplay->Display().ValueUnitVisible();
}

bool BCI2000Viewer::ToggleUnit_Enabled() const
{
    return mNumSignalChannels > 0;
}

void BCI2000Viewer::HPOff()
{
    mFilter.HPCorner(0.0);
    mRemoveMean = false;
    FilterChanged();
}

bool BCI2000Viewer::HPOff_Checked() const
{
    return mFilter.HPCorner() == 0 && !mRemoveMean;
}

void BCI2000Viewer::HPRemoveMean()
{
    mFilter.HPCorner(0.0);
    mRemoveMean = true;
    FilterChanged();
}

bool BCI2000Viewer::HPRemoveMean_Checked() const
{
    return mFilter.HPCorner() == 0 && mRemoveMean;
}

void BCI2000Viewer::HP1Hz()
{
    mFilter.HPCorner(1.0 / mSamplingRate);
    mRemoveMean = false;
    FilterChanged();
}

bool BCI2000Viewer::HP1Hz_Checked() const
{
    return mFilter.HPCorner() * mSamplingRate == 1 && !mRemoveMean;
}

void BCI2000Viewer::HP5Hz()
{
    mFilter.HPCorner(5.0 / mSamplingRate);
    mRemoveMean = false;
    FilterChanged();
}

bool BCI2000Viewer::HP5Hz_Checked() const
{
    return mFilter.HPCorner() * mSamplingRate == 5 && !mRemoveMean;
}

void BCI2000Viewer::HPCustom()
{
    if (GetCustomFilter("High Pass", "HP", &DisplayFilter::HPCorner, &DisplayFilter::HPCorner)) {
        mRemoveMean = false;
        FilterChanged();
    }
}

bool BCI2000Viewer::HPCustom_Checked() const
{
    for (QAction* pAction : {
        ui->actionHP_Off,
        ui->actionHP_Remove_Mean,
        ui->actionHP_1Hz,
        ui->actionHP_5Hz,
    })
        if (pAction->isChecked())
            return false;
    return true;
}

void BCI2000Viewer::LPOff()
{
    mFilter.LPCorner(0.0);
    FilterChanged();
}

bool BCI2000Viewer::LPOff_Checked() const
{
    return mFilter.LPCorner() == 0;
}

void BCI2000Viewer::LP30Hz()
{
    mFilter.LPCorner(30.0 / mSamplingRate);
    FilterChanged();
}

bool BCI2000Viewer::LP30Hz_Checked() const
{
    return mFilter.LPCorner() * mSamplingRate == 30;
}

void BCI2000Viewer::LP40Hz()
{
    mFilter.LPCorner(40.0 / mSamplingRate);
    FilterChanged();
}

bool BCI2000Viewer::LP40Hz_Checked() const
{
    return mFilter.LPCorner() * mSamplingRate == 40;
}

void BCI2000Viewer::LP70Hz()
{
    mFilter.LPCorner(70.0 / mSamplingRate);
    FilterChanged();
}

bool BCI2000Viewer::LP70Hz_Checked() const
{
    return mFilter.LPCorner() * mSamplingRate == 70;
}

void BCI2000Viewer::LPCustom()
{
    if (GetCustomFilter("Low Pass", "LP", &DisplayFilter::LPCorner, &DisplayFilter::LPCorner))
        FilterChanged();
}

bool BCI2000Viewer::LPCustom_Checked() const
{
    for (QAction* pAction : {
        ui->actionLP_Off,
        ui->actionLP_30Hz,
        ui->actionLP_40Hz,
        ui->actionLP_70Hz,
        })
        if (pAction->isChecked())
            return false;
    return true;
}

void BCI2000Viewer::NotchOff()
{
    mFilter.NotchCenter(0.0);
    FilterChanged();
}

void BCI2000Viewer::Notch50Hz()
{
    mFilter.NotchCenter(50.0 / mSamplingRate);
    FilterChanged();
}

void BCI2000Viewer::Notch60Hz()
{
    mFilter.NotchCenter(60.0 / mSamplingRate);
    FilterChanged();
}

bool BCI2000Viewer::Filter_Enabled() const
{
    return mSamplingRate > 0.0;
}

bool BCI2000Viewer::GetCustomFilter(
    const char* inName, const char* inAbbrev,
    double (DisplayFilter::* inGetter)() const,
    DisplayFilter& (DisplayFilter::* inSetter)(double))
{
    bool ok = false;
    QInputDialog dialog(this);
    dialog.setWindowTitle("Custom " + QString(inName));
    dialog.setLabelText("Enter a corner frequency in Hz, with or without unit");
    dialog.setInputMode(QInputDialog::TextInput);
    PhysicalUnit unit;
    unit.SetGain(mSamplingRate).SetOffset(0).SetSymbol("Hz");
    std::string s = unit.RawToPhysical((mFilter.*inGetter)()).Utf8();
    QString value = QString::fromUtf8(s.c_str());
    dialog.setTextValue(value);
    this->releaseKeyboard();
    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        value = dialog.textValue().trimmed();
        if (!value.isEmpty()) {
            double number = 0;
            if (unit.IsPhysical(value.toUtf8().constData())) {
                number = unit.PhysicalToRaw(value.toUtf8().constData());
            }
            else {
                number = value.toDouble() / mSamplingRate;
            }
            if (number <= 0 || number > 0.5) {
                bcierr << inAbbrev << " corner frequency is out of range:\n" << value.toUtf8().constData() << ".";
            }
            else {
                (mFilter.*inSetter)(number);
                ok = true;
            }
        }
    }
    this->grabKeyboard();
    return ok;
}

void BCI2000Viewer::ShowSelectedChannels()
{
    ui->channelList->blockSignals(true);
    QList<QListWidgetItem *> s = ui->channelList->selectedItems();
    for (QList<QListWidgetItem *>::iterator i = s.begin(); i != s.end(); ++i)
        (*i)->setCheckState(Qt::Checked);
    this->on_channelList_itemChanged(NULL);
    ui->channelList->blockSignals(false);
}

void BCI2000Viewer::HideSelectedChannels()
{
    ui->channelList->blockSignals(true);
    QList<QListWidgetItem *> s = ui->channelList->selectedItems();
    for (QList<QListWidgetItem *>::iterator i = s.begin(); i != s.end(); ++i)
        (*i)->setCheckState(Qt::Unchecked);
    this->on_channelList_itemChanged(NULL);
    ui->channelList->blockSignals(false);
}

bool BCI2000Viewer::ChannelsSelected() const
{
    return ui->channelList->selectedItems().size() > 0;
}

void BCI2000Viewer::HelpOnState()
{
    QListWidgetItem *pItem = ui->channelList->currentItem();
    if (pItem)
    {
        std::string name = pItem->text().toLocal8Bit().constData();
        if (ExecutableHelp().StateHelp().Exists(name))
            ExecutableHelp().StateHelp().Open(name);
        else
            QApplication::beep();
    }
}

bool BCI2000Viewer::StateSelected() const
{
    int currentRow = ui->channelList->currentRow();
    bool isStateEntry = false;
    if (currentRow > 0)
    {
        int i = currentRow;
        QListWidgetItem *p;
        do
        {
            p = ui->channelList->item(i++);
        } while (p->flags() & Qt::ItemIsUserCheckable && i < ui->channelList->count());
        isStateEntry = (i < ui->channelList->count());
    }
    if (isStateEntry)
    {
        ui->actionState_Help->setText(QString(tr("Help on %1")).arg(ui->channelList->currentItem()->text()));
        ui->actionState_Help->setEnabled(true);
    }
    else
    {
        ui->actionState_Help->setText(tr("BCI2000 Help"));
        ui->actionState_Help->setDisabled(true);
    }
    return isStateEntry;
}

void BCI2000Viewer::ToggleNotes()
{
    mpNotesWidget->setVisible(!mpNotesWidget->isVisible());
}

bool BCI2000Viewer::ToggleNotes_Enabled() const
{
    return mpNotesState != nullptr;
}

bool BCI2000Viewer::ToggleNotes_Checked() const
{
    return mpNotesWidget->isVisible();
}


void BCI2000Viewer::ToggleStatesChannels()
{
    ui->channelList->setVisible(!ui->channelList->isVisible());
}

bool BCI2000Viewer::ToggleStatesChannels_Enabled() const
{
    return true;
}

bool BCI2000Viewer::ToggleStatesChannels_Checked() const
{
    return ui->channelList->isVisible();
}

// Internal functions
//----------------------------------------------------------------------------
void BCI2000Viewer::DoFileOpen(const QString &inName)
{
    mChannelsSelected.clear();
    CloseNotes();
    DestroyAVObjects();
    DestroySecondaryViewer();
    if (mIsPrimary)
    {
        SaveSettings();
        mIsPrimary = false;
        ReadSettings();
    }
    std::string name = inName.toLocal8Bit().constData();
    mFile.Open(name.c_str());
    if (!mFile.IsOpen())
    {
        this->setWindowTitle(QApplication::applicationName());
        ui->signalDisplay->Display().SetNumSamples(0);
        mSamplingRate = 0.0;
        mFileName = "";
        UpdateVerticalScroller();
        if (!inName.isEmpty())
        {
            QString message = QString("Could not open \n\"") + inName + "\"\nas a BCI2000 file.\n";
            QMessageBox::critical(this, QApplication::applicationName(), message);
        }
        if (mpColorSelector)
            mpColorSelector->close();
    }
    else
    {
        mFileName = name;
        this->setWindowTitle(QString("%1 - %2").arg(inName, QApplication::applicationName()));
        ValueList<std::string> units;
        for (int ch = 0; ch < mFile.SignalProperties().Channels(); ++ch)
            units.push_back(mFile.SignalProperties().ValueUnit(ch).RawToPhysical(1));
        mChannelsSelected.resize(mFile.SignalProperties().Channels(), false);
        mNumSamplesDefault = std::min<int64_t>(mFile.SamplingRate() * 10, mFile.NumSamples());
        mMinValueDefault = -100;
        mMaxValueDefault = 100;
        ui->signalDisplay->Display()
            .SetNumSamples(mNumSamplesDefault)
            .SetUnitsPerValue(1.0)
            .SetValueUnits(units)
            .SetMinValue(mMinValueDefault)
            .SetMaxValue(mMaxValueDefault)
            .SetUnitsPerSample(1.0 / mFile.SamplingRate())
            .SetSampleUnit(":s");
        mSamplingRate = mFile.SamplingRate();
    }
    FillChannelList();
    OpenNotes();
    CreateAVObjects();
    CreateSecondaryViewer();
    UpdateChannelLabels();
    SetSamplePos(0);
    ui->signalDisplay->Display().SetDisplayGroups(mFile.SignalProperties().Channels());
    UpdateVerticalScroller();
    UpdateActions();
    ApplyColorTheme();
}

void BCI2000Viewer::FillChannelList()
{
    ui->channelList->blockSignals(true);
    ui->channelList->clear();
    if (mFile.IsOpen())
    {
        QListWidgetItem *pItem = new QListWidgetItem(tr("States, Streams, Events"), ui->channelList);
        pItem->setBackground(cHeadingColor);
        pItem->setFlags(Qt::NoItemFlags);
        for (const auto &state : *mFile.States())
        {
            if (!state.Name().empty() && state.Name().front() != '_')
            {
                pItem = new QListWidgetItem(state.Name().c_str(), ui->channelList);
                pItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            }
        }

        pItem = new QListWidgetItem(tr("Channels"), ui->channelList);
        pItem->setBackground(cHeadingColor);
        pItem->setFlags(Qt::NoItemFlags);
        int base = ui->channelList->count(), channelsInFile = mFile.SignalProperties().Channels();
        if (mFile.Parameters()->Exists("ChannelNames"))
        {
            ParamRef ChannelNames = mFile.Parameter("ChannelNames");
            for (int i = 0; i < ChannelNames->NumValues() && i < channelsInFile; ++i)
            {
                pItem = new QListWidgetItem(ChannelNames(i).c_str(), ui->channelList);
                pItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            }
        }
        for (int i = ui->channelList->count() - base; i < channelsInFile; ++i)
        {
            pItem = new QListWidgetItem(QString(tr("Channel %1")).arg(i + 1), ui->channelList);
            pItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        }

        int j = 0;
        for (++j; j < ui->channelList->count() && (ui->channelList->item(j)->flags() & Qt::ItemIsUserCheckable); ++j)
            ui->channelList->item(j)->setCheckState(Qt::Unchecked);
        for (++j; j < ui->channelList->count() && (ui->channelList->item(j)->flags() & Qt::ItemIsUserCheckable); ++j)
            ui->channelList->item(j)->setCheckState(Qt::Checked);
    }
    ui->channelList->blockSignals(false);
}

const char* BCI2000Viewer::SettingsPrefix() const
{
    Assert(!(mIsPrimary && mIsSecondary));
    if (mIsPrimary)
        return "Primary ";
    if (mIsSecondary)
        return "Secondary ";
    return "";
}

void BCI2000Viewer::SaveSettings() const
{
    Settings settings(SettingsPrefix());
    settings.beginGroup("Config");
    settings.setValue("Geometry", this->saveGeometry());
    settings.setValue("Splitter", ui->splitter->saveState());
    settings.setValue("WorkingDirectory", QDir::currentPath());
    std::ostringstream oss;
    if (oss << ui->signalDisplay->Display().ChannelColors())
        settings.setValue("ChannelColors", oss.str().c_str());
    settings.setValue("ColorTheme", mColorTheme);
    settings.setValue("ShowStatesChannels", ui->channelList->isVisible() ? 1 : 0);
}

void BCI2000Viewer::ReadSettings()
{
    Settings settings(SettingsPrefix());
    settings.beginGroup("Config");
    auto geom = settings.value("Geometry");
    if (geom.isValid())
        this->restoreGeometry(settings.value("Geometry").toByteArray());
    else
        RestoreDefaultGeometry();
    ui->splitter->restoreState(settings.value("Splitter").toByteArray());
    QString wd = settings.value("WorkingDirectory").toString();
    if (!wd.isEmpty())
        QDir::setCurrent(wd);
    std::istringstream iss;
    iss.str(settings.value("ChannelColors").toString().toLocal8Bit().constData());
    ColorList colors;
    if (iss >> colors)
        ui->signalDisplay->Display().SetChannelColors(colors);
    mColorTheme = settings.value("ColorTheme", dark_).toInt();
    ui->channelList->setVisible(1 == settings.value("ShowStatesChannels", true).toInt());
}

void BCI2000Viewer::RestoreDefaultGeometry()
{
    this->show(); // make sure window frame geometry has been established
#if QT_VERSION < 0x060000
    auto pScreen = windowHandle()->screen();
#else
    auto pScreen = screen();
#endif
    QRect rect = pScreen->availableGeometry(),
          framed = frameGeometry(),
          inner = geometry();
    int titleHeight = inner.top() - framed.top();
    if (mIsPrimary)
    {
        this->move(rect.left(), rect.top());
        this->resize(rect.width(), rect.height() / 2 - titleHeight);
    }
    else if (mIsSecondary)
    {
        this->move(rect.left(), rect.top() + rect.height() / 2);
        this->resize(rect.width(), rect.height() / 2 - titleHeight);
    }
}

void BCI2000Viewer::UpdateSamplePos(bool inPropagate)
{
    ui->signalDisplay->Display().SetSampleOffset(mSamplePos);
    UpdateTimeField();
    UpdateNotes();
    UpdateAVObjects();
    if (inPropagate)
        UpdatePairedViewerPos();
    UpdateActions();
}

void BCI2000Viewer::UpdateTimeField()
{
    auto pEditPosition = findChild<QLineEdit*>("editPosition");
    if (mFile.IsOpen())
    {
        TimeValue pos = static_cast<long>((2 * mSamplePos + ui->signalDisplay->Display().NumSamples()) / 2 /
                                          mFile.SamplingRate()),
                  length = static_cast<long>(mFile.NumSamples() / mFile.SamplingRate());
        std::ostringstream oss;
        oss << pos << "/" << length;
        pEditPosition->setText(oss.str().c_str());
    }
    else
        pEditPosition->setText("");
    pEditPosition->setEnabled(mFile.IsOpen());
}

void BCI2000Viewer::UpdateChannelLabels()
{
    if (mFile.IsOpen())
    {
        std::vector<std::string> signalLabels;
        if (mFile.Parameters()->Exists("ChannelNames"))
        {
            ParamRef labelParam = mFile.Parameter("ChannelNames");
            for (int k = 0; k < labelParam->NumValues(); ++k)
                signalLabels.push_back(labelParam(k));
        }
        for (int i = static_cast<int>(signalLabels.size()); i < mFile.SignalProperties().Channels(); ++i)
        {
            std::ostringstream oss;
            oss << i + 1;
            signalLabels.push_back(oss.str());
        }

        LabelList channelLabels;
        int numMarkerChannels = 0;
        int j = 1;
        while (j < ui->channelList->count() && (ui->channelList->item(j)->flags() & Qt::ItemIsUserCheckable))
            ++j;
        int chBase = ++j;
        for (; j < ui->channelList->count() && (ui->channelList->item(j)->flags() & Qt::ItemIsUserCheckable); ++j)
            channelLabels.push_back(Label(channelLabels.size(), signalLabels[j - chBase]));
        for (int i = 1; i < ui->channelList->count() && (ui->channelList->item(i)->flags() & Qt::ItemIsUserCheckable);
             ++i)
            if (ui->channelList->item(i)->checkState() == Qt::Checked)
            {
                channelLabels.push_back(Label(channelLabels.size(),
                                              ui->channelList->item(i)->text().toLocal8Bit().constData()));
                ++numMarkerChannels;
            }
        mNumSignalChannels = static_cast<int>(channelLabels.size()) - numMarkerChannels;
        ui->signalDisplay->Display()
            .SetNumMarkerChannels(numMarkerChannels)
            .SetChannelLabels(channelLabels)
            .SetChannelLabelsVisible(true);
    }
    else
    {
        mNumSignalChannels = 0;
        ui->signalDisplay->Display().SetNumMarkerChannels(0).SetChannelLabels(LabelList());
    }
}

void BCI2000Viewer::UpdateVerticalScroller()
{
    ui->verticalScrollBar->blockSignals(true);
    int numChannels = mNumSignalChannels, pageSize = ui->signalDisplay->Display().DisplayGroups();
    if (pageSize < 1)
        pageSize = 1;
    int scrollMax = numChannels - pageSize;
    if (scrollMax < 0)
        scrollMax = 0;
    if (pageSize > scrollMax)
    {
        ui->verticalScrollBar->setMinimum(0);
        ui->verticalScrollBar->setMaximum(0);
        ui->verticalScrollBar->setEnabled(false);
    }
    else
    {
        ui->verticalScrollBar->setPageStep(pageSize);
        ui->verticalScrollBar->setSingleStep(1);
        ui->verticalScrollBar->setMinimum(0);
        ui->verticalScrollBar->setMaximum(scrollMax);
        ui->verticalScrollBar->setSliderPosition(ui->signalDisplay->Display().TopGroup());
        ui->verticalScrollBar->setEnabled(true);
    }
    ui->verticalScrollBar->blockSignals(false);
}

GenericSignal BCI2000Viewer::ConstructDisplaySignal(int64_t inPos, int64_t inLength)
{
    GenericSignal result;
    if (mFile.IsOpen())
    {
        int64_t startTime = QDateTime::currentMSecsSinceEpoch();
        bool setCursor = mPrevReadSignalDuration > 100;
        if (setCursor)
            QApplication::setOverrideCursor(Qt::WaitCursor);
        int i = 1;
        std::vector<StateRef> states;
        for (; i < ui->channelList->count() && (ui->channelList->item(i)->flags() & Qt::ItemIsUserCheckable); ++i)
            if (ui->channelList->item(i)->checkState() == Qt::Checked)
                states.push_back(mFile.State(ui->channelList->item(i)->text().toLocal8Bit().constData()));
        std::vector<int> channels;
        int base = ++i;
        for (; i < ui->channelList->count() && (ui->channelList->item(i)->flags() & Qt::ItemIsUserCheckable); ++i)
            channels.push_back(i - base);

        GenericSignal signal(channels.size() + states.size(), inLength), statevalues(states.size(), inLength);
        GenericSignal::ValueType* pSignal = signal.MutableData(GenericSignal::intentOverwriteContents);
        GenericSignal::ValueType* pStatevalues = statevalues.MutableData(GenericSignal::intentOverwriteContents);
        int64_t sampleInFile = inPos;
        for (int64_t sample = 0; sample < signal.Elements() && sampleInFile < mFile.NumSamples();
             ++sample, ++sampleInFile)
        {
            for (int channelIdx = 0; channelIdx < channels.size(); ++channelIdx)
                pSignal[signal.LinearIndex(channelIdx, sample)] = mFile.CalibratedValue(channels[channelIdx], sampleInFile);

            mFile.ReadStateVector(sampleInFile);
            for (size_t i = 0; i < states.size(); ++i)
                pStatevalues[statevalues.LinearIndex(i, sample)] = states[i].AsUnsigned();
        }

        if (FilterActive())
        {
            mFilter.Reset();
            result = GenericSignal(signal.Properties());
            // run the filter twice to avoid transient artifacts
            mFilter.Process(signal, result);
            mFilter.Process(signal, result);
        }
        else
        {
            result = signal;
        }

        GenericSignal::ValueType* pResult = result.MutableData(GenericSignal::intentRetainContents);
        if (mRemoveMean)
        {
            for (size_t ch = 0; ch < channels.size(); ++ch)
            {
                double mean = 0.0;
                for (int sample = 0; sample < result.Elements(); ++sample)
                    mean += result(ch, sample);
                mean /= result.Elements();
                for (int sample = 0; sample < result.Elements(); ++sample)
                    pResult[result.LinearIndex(ch, sample)] -= mean;
            }
        }

        for (int ch = 0; ch < statevalues.Channels(); ++ch)
            for (int sample = 0; sample < statevalues.Elements(); ++sample)
                pResult[result.LinearIndex(ch + channels.size(), sample)] = pStatevalues[statevalues.LinearIndex(ch, sample)];

        mFile.ReadStateVector(inPos + inLength - 1);
        mPrevReadSignalDuration = QDateTime::currentMSecsSinceEpoch() - startTime;
        if (setCursor)
            QApplication::restoreOverrideCursor();
    }
    else
    {
        result = GenericSignal(0, 0);
    }
    return result;
}

int64_t BCI2000Viewer::NumSamples() const
{
    return mFile.NumSamples();
}

int64_t BCI2000Viewer::NumDisplaySamples() const
{
    return ui->signalDisplay->Display().NumSamples();
}

void BCI2000Viewer::SetNumDisplaySamples(int inSamples, bool inPropagate)
{
    if (inSamples > mFile.NumSamples())
        inSamples = mFile.NumSamples();
    ui->signalDisplay->Display().SetNumSamples(inSamples);
    if (inPropagate)
        UpdatePairedViewerSampleZoom();
    SetSamplePos(mSamplePos, false);
}

void BCI2000Viewer::SetSamplePos(int64_t inPos, bool inPropagate)
{
    if (mFile.IsOpen())
    {
        mSamplePos = inPos;
        if (mSamplePos >= mFile.NumSamples() - NumDisplaySamples())
            mSamplePos = mFile.NumSamples() - NumDisplaySamples() - 1;
        if (mSamplePos < 0)
            mSamplePos = 0;
        GenericSignal signal = ConstructDisplaySignal(mSamplePos, NumDisplaySamples());
        ui->signalDisplay->Display().WrapForward(signal);
    }
    else
    {
        ui->signalDisplay->Display().SetNumSamples(0).WrapForward(GenericSignal(0, 0));
        mSamplePos = 0;
    }
    UpdateSamplePos(inPropagate);
}

void BCI2000Viewer::ScrollSamplePosBy(int64_t inPosDiff, bool inPropagate)
{
    if (mSamplePos + inPosDiff < 0)
    {
        SetSamplePos(0, inPropagate);
        return;
    }
    if (mSamplePos + inPosDiff >= mFile.NumSamples() - NumDisplaySamples())
    {
        SetSamplePos(LastSample(), inPropagate);
        return;
    }
    if (inPosDiff != 0 && mFile.IsOpen())
    {
        if (FilterActive() || mRemoveMean)
        {
            SetSamplePos(mSamplePos + inPosDiff, inPropagate);
        }
        else
        {
            int64_t readCursor = inPosDiff > 0 ? mSamplePos + NumDisplaySamples() : mSamplePos + inPosDiff;
            int64_t length = std::min(::abs(inPosDiff), NumDisplaySamples());
            GenericSignal signal = ConstructDisplaySignal(readCursor, length);
            mSamplePos += inPosDiff;
            if (inPosDiff > 0)
                ui->signalDisplay->Display().ScrollForward(signal);
            else
                ui->signalDisplay->Display().ScrollBack(signal);
            UpdateSamplePos(inPropagate);
        }
    }
}

void BCI2000Viewer::ScrollToSamplePos(int64_t inPos, bool inPropagate)
{
    int64_t delta = inPos - mSamplePos;
    if (delta != 0)
        ScrollSamplePosBy(delta, inPropagate);
}

void BCI2000Viewer::FilterChanged()
{
    SetSamplePos(mSamplePos, false);
}

bool BCI2000Viewer::FilterActive()
{
    return (mFilter.HPCorner() != 0.0) || (mFilter.LPCorner() != 0.0) || (mFilter.NotchCenter() != 0.0);
}

namespace {

    QToolButton* CreateToolButton(const QString& title, QMenu* pMenu, const QString& tooltip = "")
    {
        QToolButton* pToolButton = new QToolButton;
        pToolButton->setText(title);
        pToolButton->setToolTip(tooltip.isEmpty() ? title : tooltip);
        pToolButton->setPopupMode(QToolButton::InstantPopup);
        pToolButton->setMenu(pMenu);
        return pToolButton;
    }

}

void BCI2000Viewer::PopulateToolbars()
{
    QToolBar* pToolbar = new QToolBar("View options");

    pToolbar->addAction(ui->actionEnlarge_Signal);
    pToolbar->addAction(ui->actionReduce_Signal);
    pToolbar->addAction(ui->actionReset);
    pToolbar->addSeparator();
    pToolbar->addAction(ui->actionZoom_In);
    pToolbar->addAction(ui->actionZoom_Out);
    pToolbar->addAction(ui->actionReset_Zoom);
    pToolbar->addSeparator();
    pToolbar->addAction(ui->actionMore_Channels);
    pToolbar->addAction(ui->actionFewer_Channels);
    pToolbar->addSeparator();
    pToolbar->addAction(ui->actionShow_Grid);
    pToolbar->addAction(ui->actionChoose_Channel_Colors);

    mpColorThemeButton = CreateToolButton("Color Theme", ui->menuColor_Theme);
    pToolbar->addWidget(mpColorThemeButton);
    pToolbar->addSeparator();

    mpHPButton = CreateToolButton("HP", ui->menuHigh_Pass, "High Pass");
    pToolbar->addWidget(mpHPButton);
    mpLPButton = CreateToolButton("LP", ui->menuLow_Pass, "Low Pass");
    pToolbar->addWidget(mpLPButton);
    mpNotchButton = CreateToolButton("Notch", ui->menuNotch);
    pToolbar->addWidget(mpNotchButton);

    addToolBar(pToolbar);

    pToolbar = new QToolBar("Navigation");

    pToolbar->addAction(findChild<QAction*>("actionToBegin"));
    pToolbar->addAction(findChild<QAction*>("actionPageRew"));
    pToolbar->addAction(findChild<QAction*>("actionBlockRew"));
    QLineEdit* pEditPosition = new QLineEdit;
    pEditPosition->setObjectName("editPosition");
    pEditPosition->setMinimumSize(110, 0);
    pEditPosition->setMaximumWidth(110);
    pEditPosition->setAlignment(Qt::AlignCenter);
    connect(pEditPosition, &QLineEdit::textEdited, this, &BCI2000Viewer::OnEditPositionTextEdited);
    connect(pEditPosition, &QLineEdit::selectionChanged, this, &BCI2000Viewer::OnEditPositionSelectionChanged);
    connect(pEditPosition, &QLineEdit::returnPressed, this, &BCI2000Viewer::OnEditPositionReturnPressed);
    connect(pEditPosition, &QLineEdit::editingFinished, this, &BCI2000Viewer::OnEditPositionEditingFinished);
    pToolbar->addWidget(pEditPosition);
    pToolbar->addAction(findChild<QAction*>("actionBlockFwd"));
    pToolbar->addAction(findChild<QAction*>("actionPageFwd"));
    pToolbar->addAction(findChild<QAction*>("actionToEnd"));

    addToolBar(pToolbar);
}

namespace {
    QPixmap MirrorIcon(const QIcon& inIcon)
    {
        QImage image1 = inIcon.pixmap(inIcon.availableSizes().back()).toImage();
        QImage image2(image1.size(), image1.format());
        for (int x = 0; x < image1.width(); ++x) {
            for (int y = 0; y < image1.height(); ++y) {
                image2.setPixel(x, y, image1.pixel(image1.width() - x - 1, y));
            }
        }
        return QPixmap::fromImage(image2);
    }
}

void BCI2000Viewer::LoadActionIcons()
{
    ui->actionEnlarge_Signal->setIcon(LoadIcon(Resources::zoom_in_v_icon));
    ui->actionReduce_Signal->setIcon(LoadIcon(Resources::zoom_out_v_icon));
    ui->actionReset->setIcon(LoadIcon(Resources::zoom_reset_v_icon));

    ui->actionZoom_In->setIcon(LoadIcon(Resources::zoom_in_h_icon));
    ui->actionZoom_Out->setIcon(LoadIcon(Resources::zoom_out_h_icon));
    ui->actionReset_Zoom->setIcon(LoadIcon(Resources::zoom_reset_h_icon));

    ui->actionMore_Channels->setIcon(LoadIcon(Resources::plus_icon));
    ui->actionFewer_Channels->setIcon(LoadIcon(Resources::minus_icon));
    ui->actionShow_Grid->setIcon(LoadIcon(Resources::grid_icon));
    ui->actionChoose_Channel_Colors->setIcon(LoadIcon(Resources::colorwheel_icon));
    ui->menuColor_Theme->setIcon(LoadIcon(Resources::light_dark_icon));
    mpColorThemeButton->setIcon(LoadIcon(Resources::light_dark_icon));

    findChild<QAction*>("actionToBegin")->setIcon(LoadIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward)));
    findChild<QAction*>("actionPageRew")->setIcon(LoadIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward)));
    findChild<QAction*>("actionBlockRew")->setIcon(LoadIcon(MirrorIcon(style()->standardIcon(QStyle::SP_MediaPlay))));
    findChild<QAction*>("actionBlockFwd")->setIcon(LoadIcon(style()->standardIcon(QStyle::SP_MediaPlay)));
    findChild<QAction*>("actionPageFwd")->setIcon(LoadIcon(style()->standardIcon(QStyle::SP_MediaSeekForward)));
    findChild<QAction*>("actionToEnd")->setIcon(LoadIcon(style()->standardIcon(QStyle::SP_MediaSkipForward)));
}

namespace {

    void InvertIcon(QPixmap& icon) // only works for greyscale images
    {
        QImage img = icon.toImage();
        for (int x = 0; x < img.width(); ++x) {
            for (int y = 0; y < img.height(); ++y) {
                QColor color = img.pixelColor(x, y);
                int h, s, v, a;
                color.getHsv(&h, &s, &v, &a);
                v = 255 - v;
                img.setPixelColor(x, y, QColor::fromHsv(h, s, v, a));
            }
        }
        icon = QPixmap::fromImage(img);
    }

} // namespace

QPixmap BCI2000Viewer::LoadIcon(const bci::Resource& res, bool adapt)
{
    QPixmap icon;
    icon.loadFromData(reinterpret_cast<const uchar*>(res.data), res.length);
    if (adapt && IsPaletteDark(this->palette()))
        InvertIcon(icon);
    return icon;
}

QPixmap BCI2000Viewer::LoadIcon(const QIcon& inIcon, bool adapt)
{
    QPixmap icon = inIcon.pixmap(inIcon.availableSizes().back());
    if (adapt && IsPaletteDark(this->palette()))
        InvertIcon(icon);
    return icon;
}



void BCI2000Viewer::OpenNotes()
{
    // Look for a notes file and open it if present
    std::string base = FileUtils::StripExtension(mFileName),
                extpart = FileUtils::ExtractExtension(base); // second extension if present
    base = FileUtils::StripExtension(base); // remove second extension if present
    std::string notesFile = base + "_notes" + extpart + ".txt";
    if (FileUtils::IsFile(notesFile) && mFile.States()->Exists("Notes"))
    {
        mpNotesState = new StateRef(mFile.State("Notes"));
        mpNotesWidget->setNotesFile(notesFile);
        mpNotesWidget->setVisible(true);
    }
    else
    {
        CloseNotes();
    }
}

void BCI2000Viewer::UpdateNotes()
{
    if (mFile.IsOpen() && mpNotesState)
    {
        mFile.ReadStateVector(mSamplePos + NumDisplaySamples() - 1);
        mpNotesWidget->setLinePosition(*mpNotesState);
    }
}

void BCI2000Viewer::CloseNotes()
{
    mpNotesWidget->setNotesFile("");
    mpNotesWidget->setVisible(false);
    delete mpNotesState;
    mpNotesState = nullptr;
}

void BCI2000Viewer::SyncWithNote(int note)
{
    if (mpNotesState && mFile.IsOpen())
    {
        int64_t pos = mSamplePos + NumDisplaySamples() - 1;
        while (pos > NumDisplaySamples() && *mpNotesState >= note + 1)
            mFile.ReadStateVector(pos--);
        while (pos < NumSamples() && *mpNotesState < note + 1)
            mFile.ReadStateVector(pos++);
        SetSamplePos(pos + 1 - NumDisplaySamples());
    }
}

void BCI2000Viewer::Play()
{
    mLastTimerStamp = QDateTime::currentMSecsSinceEpoch();
    mPlaybackTimer.start();
    for (const auto& p : mAudioPlayers) {
        p->Play();
        p->AdjustPlaybackTempo((mPlaybackRatio));
    }
}

void BCI2000Viewer::Pause()
{
    mPlaybackTimer.stop();
    for (const auto& p : mAudioPlayers)
        p->Pause();
}

void BCI2000Viewer::SetVolume(float volume)
{
    for (const auto& p : mAudioPlayers)
        p->SetVolume(volume);
}

void BCI2000Viewer::SetPlaybackRatio(float f)
{
    mPlaybackRatio = f;
}

bool BCI2000Viewer::Playing() const
{
    return mPlaybackTimer.isActive();
}

void BCI2000Viewer::PlaybackTimer()
{
    int64_t samples = 0;
    int64_t now = QDateTime::currentMSecsSinceEpoch();
    samples = mPlaybackRatio * ((now - mLastTimerStamp) * mFile.SamplingRate() / 1000);
    mLastTimerStamp = now;
    ScrollSamplePosBy(samples);
    if (!GoForward_Enabled())
        Pause();
    UpdateActions();
}

void BCI2000Viewer::CreateAVObjects()
{
    if (mIsSecondary)
        return;

    Settings settings;
    settings.beginGroup("Config");

    mpPlaybackControl = new PlaybackControl(this);
    mpPlaybackControl->move(10, 60);
    mpPlaybackControl->restoreGeometry(settings.value("PlaybackControl").toByteArray());
    ui->menuWindow->setEnabled(true);
    QAction *pAction = ui->menuWindow->addAction("Playback Control");
    connect(pAction, &QAction::triggered, this, &BCI2000Viewer::BringPlaybackControlToFront);

    std::string base = FileUtils::StripExtension(mFileName),
                extpart = FileUtils::ExtractExtension(base); // second extension if present
    base = FileUtils::StripExtension(base); // remove second extension if present

    // setup common audio options
    int audioBufferSize = cAudioExtensionDefaultBufferSize;
    if (mFile.Parameters()->Exists("AudioBufferSize"))
        audioBufferSize = mFile.Parameter("AudioBufferSize");
    double audioBufferDelayMs = audioBufferSize * 1000.0 / cAudioExtensionSamplingRate_Hz;

    // first, look for webcam-related states and files
    for (int i = 0; i < cMaxWebcams; ++i)
    {
        std::string name = "WebcamFrame" + std::to_string(i);
        if (mFile.States()->Exists(name))
        {
            std::string videoName = base + "_" + std::to_string(i) + "_vid" + extpart + ".mp4";
            if (FileUtils::IsFile(videoName))
            {
                VideoWidget *pWidget = new VideoWidget(this);
                pWidget->setVideoFile(videoName);
                pWidget->setWindowTitle(QString("Webcam %1").arg(i));
                VideoLink link = { mFile.State(name), pWidget };
                mVideoWidgets.push_back(link);
            }
        }
    }

    // then, look for Screen Capture related states and files
    int j = 1;
    std::string stateName = "ScreenCaptureFrame1";
    while (mFile.States()->Exists(stateName)) {

        std::string videoName = base + "_ScreenCapture" + std::to_string(j) + extpart + ".mp4";
        if (FileUtils::IsFile(videoName))
        {
            VideoWidget* pWidget = new VideoWidget(this);
            pWidget->setVideoFile(videoName);
            pWidget->setWindowTitle(QString("Screen Capture %1").arg(j));
            VideoLink link = { mFile.State(stateName), pWidget };
            mVideoWidgets.push_back(link);
        }
        stateName = "ScreenCaptureFrame" + std::to_string(++j);
    }

    // then, look for VideoStreaming related states and files
    for (std::string origin : {"Local", "Remote"})
    {
        std::string name = "VideoStreaming" + origin;
        std::string statename = name + "Frame";
        if (mFile.States()->Exists(statename))
        {
            std::string videoName = base + "_" + name + extpart + ".mp4";
            if (FileUtils::IsFile(videoName))
            {
                VideoWidget* pWidget = new VideoWidget(this);
                pWidget->setVideoFile(videoName);
                pWidget->setWindowTitle(QString::fromLocal8Bit(("Video Streaming " + origin).c_str()));
                VideoLink link = { mFile.State(statename), pWidget };
                mVideoWidgets.push_back(link);
            }
        }

        name = "AudioStreaming" + origin;
        statename = name + "Frame";
        if (mFile.States()->Exists(statename))
        {
            std::string audioName = base + "_" + name + extpart + ".aac";
            if (FileUtils::IsFile(audioName))
            {
                auto pAudioPlayer = std::make_unique<FrameControlledAudioPlayer>(mpAudioSink, mFile.State(statename), 0);
                pAudioPlayer->SetFile(audioName);
                pAudioPlayer->SetExternalPlaybackLatency(Time::FromMilliseconds(audioBufferDelayMs));
                mAudioPlayers.push_back(std::move(pAudioPlayer));
            }
        }
    }

    // then, look for recorded visualizations and their states
    const std::string stateMark = "VisFrame_";
    for (const auto& state : *mFile.States())
    {
        if (state.Name().find(stateMark) == 0)
        {
            std::string visID = state.Name().substr(stateMark.length());
            std::string videoName = base + "_" + visID + extpart + ".mp4";
            if (FileUtils::IsFile(videoName))
            {
                VideoWidget* pWidget = new VideoWidget(this);
                pWidget->setVideoFile(videoName);
                pWidget->setWindowTitle(QString::fromLocal8Bit(visID.c_str()));
                VideoLink link = { mFile.State(state.Name()), pWidget };
                mVideoWidgets.push_back(link);
            }
        }
    }

    int i = 0;
    for (const auto &w : mVideoWidgets)
    {
        QAction *pAction = ui->menuWindow->addAction(w.widget->windowTitle());
        pAction->setProperty("Window", i);
        connect(pAction, SIGNAL(triggered()), this, SLOT(BringVideoWindowToFront()));
        w.widget->setWindowFlags(Qt::Tool);
        w.widget->resize(w.widget->width() / 2, w.widget->height() / 2);
        w.widget->restoreGeometry(settings.value(QString("Webcam%1").arg(i)).toByteArray());
        w.widget->show();
        ++i;
    }

    if (mFile.States()->Exists("AudioFrame"))
    {
        StateRef audioFrame = mFile.State("AudioFrame");
        mFile.ReadStateVector(0);
        int64_t initialValue = audioFrame;
        int64_t i = 1;
        while (i < mFile.NumSamples() && audioFrame == initialValue)
            mFile.ReadStateVector(i++);
        double delta_t = i / mFile.SamplingRate();
        double delta_audioFrames = delta_t * cAudioExtensionSamplingRate_Hz;
        int64_t audioFrameOffset = audioFrame - delta_audioFrames;
        mFile.ReadStateVector(0);
        std::string audioInName = base + "_in" + extpart,
                    audioOutName = base + "_out" + extpart;
        for (const char *ext : {".wav", ".flac", ".ogg"})
        {
            if (FileUtils::IsFile(audioInName + ext))
            {
                auto pAudioPlayer = std::make_unique<FrameControlledAudioPlayer>(mpAudioSink, audioFrame, audioFrameOffset);
                pAudioPlayer->SetFile(audioInName + ext);
                pAudioPlayer->SetExternalPlaybackLatency(Time::FromMilliseconds(audioBufferDelayMs));
                mAudioPlayers.push_back(std::move(pAudioPlayer));
            }
            if (FileUtils::IsFile(audioOutName + ext))
            {
                auto pAudioPlayer = std::make_unique<FrameControlledAudioPlayer>(mpAudioSink, audioFrame, audioFrameOffset);
                pAudioPlayer->SetFile(audioOutName + ext);
                pAudioPlayer->SetExternalPlaybackLatency(Time::FromMilliseconds(audioBufferDelayMs));
                mAudioPlayers.push_back(std::move(pAudioPlayer));
            }
        }
    }

    if (mFile.IsOpen() || !mVideoWidgets.empty() || !mAudioPlayers.empty())
        mpPlaybackControl->show();
}

void BCI2000Viewer::UpdateAVObjects()
{
    if (mFile.IsOpen())
        mFile.ReadStateVector(mSamplePos + NumDisplaySamples() - 1);

    for (const auto &entry : mVideoWidgets) {
        int64_t position = entry.state;
        if (mSamplePos == 0)
            position = 0;
        entry.widget->setFramePosition(position);
    }

    for (const auto& p : mAudioPlayers) {
        if (p->IsPlaying()) {
            p->AdjustPlaybackTempo(mPlaybackRatio);
        }
        else {
            p->SyncFramePosition();
        }
    }

    if (mpPlaybackControl)
        mpPlaybackControl->updatePosition(mSamplePos);
}

void BCI2000Viewer::DestroyAVObjects()
{
    Settings settings;
    settings.beginGroup("Config");
    int i = 0;
    for (auto &w : mVideoWidgets)
    {
        settings.setValue(QString("Webcam%1").arg(i++), w.widget->saveGeometry());
        delete w.widget;
    }
    mVideoWidgets.clear();
    mAudioPlayers.clear();

    ui->menuWindow->clear();
    ui->menuWindow->setEnabled(false);

    if (mpPlaybackControl)
    {
        settings.setValue(QString("PlaybackControl"), mpPlaybackControl->saveGeometry());
        delete mpPlaybackControl;
        mpPlaybackControl = nullptr;
    }
}

void BCI2000Viewer::SyncStatevectorForAudio()
{
    mFile.ReadStateVector(mSamplePos + NumDisplaySamples() - 1);
}

void BCI2000Viewer::BringPlaybackControlToFront()
{
    if (mpPlaybackControl)
    {
        mpPlaybackControl->show();
        mpPlaybackControl->raise();
        mpPlaybackControl->activateWindow();
    }
}

void BCI2000Viewer::BringVideoWindowToFront()
{
    auto property = sender()->property("Window");
    if (property.isValid())
    {
        bool ok = false;
        int i = property.toInt(&ok);
        if (ok && i >= 0 && i < mVideoWidgets.size())
        {
            QWidget *w = mVideoWidgets[i].widget;
            w->show();
            w->raise();
            w->activateWindow();
        }
    }
}

std::string BCI2000Viewer::GetSecondaryFile(const std::string& inPrimaryFile)
{
    std::string base = FileUtils::StripExtension(inPrimaryFile),
                ext1 = FileUtils::ExtractExtension(inPrimaryFile),
                ext2 = FileUtils::ExtractExtension(base);
    base = FileUtils::StripExtension(base);
    std::string secondaryFile = base + "_1" + ext2 + ext1;
    return secondaryFile;
}

void BCI2000Viewer::CreateSecondaryViewer()
{
    Assert(!mpSecondaryViewer);
    std::string secondaryFile = GetSecondaryFile(mFileName);
    if (FileUtils::IsFile(secondaryFile))
    {
        if (!mIsPrimary)
        {
            SaveSettings();
            mIsPrimary = true;
            ReadSettings();
        }
        secondaryFile = "\"" + secondaryFile + "\"";
        mpSecondaryViewer = new ChildProcess(FileUtils::ExecutablePath(), "--secondary " + secondaryFile);
        mpSecondaryViewer->run();
        // defer start of IO thread until events are being processed
        QTimer::singleShot(0, this, &BCI2000Viewer::OnIOThreadTimer);
    }
}

void BCI2000Viewer::UpdatePairedViewerPos()
{
    if (!mIsPrimary && !mIsSecondary)
        return;

    std::ostream output(mpSecondaryViewer ? &mpSecondaryViewer->standardIO() : std::cout.rdbuf());
    output.precision(std::numeric_limits<double>::max_digits10);
    output << "pos " << (mSamplePos / mSamplingRate) << std::endl;
}

void BCI2000Viewer::UpdatePairedViewerSampleZoom()
{
    if (!mIsPrimary && !mIsSecondary)
        return;

    std::ostream output(mpSecondaryViewer ? &mpSecondaryViewer->standardIO() : std::cout.rdbuf());
    output.precision(std::numeric_limits<double>::max_digits10);
    output << "width " << (NumDisplaySamples() / mSamplingRate) << std::endl;
}

void BCI2000Viewer::DestroySecondaryViewer()
{
    if (mpSecondaryViewer && mpSecondaryViewer->running())
    {
        Tiny::OStream output(mpSecondaryViewer->standardIO());
        output << "quit" << std::endl;
        mpSecondaryViewer->Wait();
    }
    delete mpSecondaryViewer;
    mpSecondaryViewer = nullptr;
}

void BCI2000Viewer::OnIOThreadTimer()
{
#if _WIN32
    if (mIsPrimary)
    {
        this->showMinimized();
        this->showNormal();
    }
#endif
    mSecondaryViewerThread = std::thread([this] {this->IOThread();});
}

void BCI2000Viewer::IOThread()
{
    std::cout << "ready" << std::endl;

    std::istream input(mpSecondaryViewer ? &mpSecondaryViewer->standardIO() : std::cin.rdbuf());
    std::string line;
    while (std::getline(input, line))
    {
        if (!line.empty() && line.back() == '\r') // Windows text mode
            line.pop_back();

        if (line.find("pos ") == 0)
        {
            double pos = std::strtod(line.c_str() + 4, nullptr);
            int64_t samplePos = pos * mSamplingRate;
            QMetaObject::invokeMethod(this, "ScrollToSamplePos", Qt::QueuedConnection, Q_ARG(int64_t, samplePos), Q_ARG(bool, false));
        }
        else if (line.find("width ") == 0)
        {
            double width = std::strtod(line.c_str() + 6, nullptr);
            int sampleWidth = width * mSamplingRate;
            QMetaObject::invokeMethod(this, "SetNumDisplaySamples", Qt::QueuedConnection, Q_ARG(int, sampleWidth), Q_ARG(bool, false));
        }
        else if (line == "ready")
        {
            ; // ignore it
        }
        else if (line == "quit")
        {
            if (mIsSecondary)
            {
                std::cout << "quit" << std::endl;
                QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
            }
            return;
        }
        else
        {
            SuggestDebugging << "unexpected input: " << line;
        }
    }
}

int64_t BCI2000Viewer::LastSample() const
{
    return std::max<int64_t>(0LL, mFile.NumSamples() - ui->signalDisplay->Display().NumSamples());
}

void BCI2000Viewer::SetChannelSelected(int ch, bool selected)
{
    Assert(ch < mChannelsSelected.size());
    mChannelsSelected[ch] = selected;
    UpdateChannelSelection();
}

void BCI2000Viewer::ExtendChannelSelection(int ch)
{
    Assert(ch < mChannelsSelected.size());
    bool anchorValid = (mSelectionAnchor >= 0 && mSelectionAnchor < mChannelsSelected.size());
    anchorValid = anchorValid && mChannelsSelected[mSelectionAnchor];
    if (anchorValid) {
        for (int i = mSelectionAnchor; i <= ch; ++i) {
            if (IsChannelVisible(i))
                mChannelsSelected[i] = true;
        }
        for (int i = ch; i <= mSelectionAnchor; ++i) {
            if (IsChannelVisible(i))
                mChannelsSelected[i] = true;
        }
    }
    else {
        mChannelsSelected[ch] = true;
    }
    mSelectionAnchor = ch;
    UpdateChannelSelection();
}

bool BCI2000Viewer::ToggleChannelSelected(int ch)
{
    Assert(ch < mChannelsSelected.size());
    mChannelsSelected[ch] = !mChannelsSelected[ch];
    UpdateChannelSelection();
    return mChannelsSelected[ch];
}

void BCI2000Viewer::UpdateChannelSelection()
{
    bool oldState = ui->channelList->blockSignals(true);
    int begin = ui->channelList->count() - mChannelsSelected.size();
    for (int row = begin; row < ui->channelList->count(); ++row) {
        ui->channelList->item(row)->setSelected(mChannelsSelected[row - begin]);
    }
    ui->channelList->blockSignals(oldState);
    ui->signalDisplay->Display().SetChannelSelected(mChannelsSelected);
    if (mpColorSelector)
        mpColorSelector->setApplyColorButtonEnabled(AnyChannelsSelected());
}

bool BCI2000Viewer::IsChannelVisible(int ch) const
{
    int begin = ui->channelList->count() - mChannelsSelected.size();
    if (ui->channelList->count() > begin + ch) {
        return ui->channelList->item(begin + ch)->checkState();
    }
    return true;
}

void BCI2000Viewer::SelectAllChannels()
{
    for (int i = 0; i < mChannelsSelected.size(); ++i) {
        mChannelsSelected[i] = true;
    }
    UpdateChannelSelection();
}

void BCI2000Viewer::ClearChannelSelection()
{
    for (int i = 0; i < mChannelsSelected.size(); ++i) {
        mChannelsSelected[i] = false;
    }
    UpdateChannelSelection();
}

bool BCI2000Viewer::AnyChannelsSelected() const
{
    for (bool b : mChannelsSelected)
        if (b)
            return true;
    return false;
}

std::vector<bool> BCI2000Viewer::ExtendedChannelSelection() const
{
    auto selected = mChannelsSelected;
    bool found = false;
    for (bool b : selected) {
        if (b) {
            found = true;
            break;
        }
    }
    if (!found) { // if nothing is selected, apply to all
        for (int ch = 0; ch < selected.size(); ++ch)
            selected[ch] = true;
    }
    return selected;
}

std::pair<int, int> BCI2000Viewer::YRangeToChannels(float y0, float y1) const
{
    if (y0 > y1)
        return YRangeToChannels(y1, y0);
    const SignalDisplay& display = ui->signalDisplay->Display();
    int count = display.NumChannels();
    int min = display.ChannelTop(0);
    int max = display.ChannelBottom(count - 1);
    if (y0 < min)
        return YRangeToChannels(min, y1);
    if (y1 > max)
        return YRangeToChannels(y0, max);
    int minChannel = display.PosToChannel(y0);
    int maxChannel = display.PosToChannel(y1);
    return std::make_pair(minChannel, maxChannel);
}

// FrameControlledAudioPlayer
BCI2000Viewer::FrameControlledAudioPlayer::FrameControlledAudioPlayer(AudioSinkRef sink, StateRef state, int64_t offset)
: TimeStretchAudioPlayer(sink), mState(state), mFrameOffset(offset)
{
}

void BCI2000Viewer::FrameControlledAudioPlayer::SyncFramePosition()
{
    SetFramePosition(mState - mFrameOffset);
}

void BCI2000Viewer::FrameControlledAudioPlayer::AdjustPlaybackTempo(double playbackRatio)
{
    int64_t targetFramePosition = mState - mFrameOffset;
    int64_t currentFramePosition = FramePosition();
    int delta = targetFramePosition - currentFramePosition;
    // tanh() limits to a range between -1 and 1
    // pow(2, tanh()) is between 0.5 and 2.0
    double adjustment = ::pow(2, ::tanh(mAlpha * delta));
    SetTempo(adjustment * playbackRatio);
}

