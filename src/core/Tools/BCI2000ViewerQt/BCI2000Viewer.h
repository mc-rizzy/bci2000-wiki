////////////////////////////////////////////////////////////////////////////////
// $Id: BCI2000Viewer.h 9418 2026-06-18 13:11:51Z mellinger $
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
#ifndef BCI2000VIEWER_H
#define BCI2000VIEWER_H

#include <QMainWindow>
#include <QtWidgets>

#include "AudioSink.h"
#include "TimeStretchAudioPlayer.h"
#include "PlaybackControl.h"
#include "BCI2000FileReader.h"
#include "Color.h"
#include "DisplayFilter.h"
#include "GenericSignal.h"
#include "VideoWidget.h"
#include "NotesWidget.h"
#include "ChildProcess.h"
#include "Resource.h"

namespace Ui
{
class BCI2000Viewer;
}

class BCI2000Viewer : public QMainWindow
{
    Q_OBJECT

  public:
    BCI2000Viewer(QWidget *parent = 0);
    ~BCI2000Viewer();

  private:
    Ui::BCI2000Viewer *ui;

  protected:
    // Qt event handlers
    void closeEvent(QCloseEvent *) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void changeEvent(QEvent *) override;
    bool event(QEvent *) override;

  private slots:
    void onFileOpen(const QString&);
    bool onSignalDisplayToolTip(const QPoint&);
    void on_signalDisplay_customContextMenuRequested(QPoint pos);
    void OnEditPositionTextEdited(QString);
    void OnEditPositionEditingFinished();
    void OnEditPositionReturnPressed();
    void OnEditPositionSelectionChanged();
    void on_channelList_itemChanged(QListWidgetItem *);
    void on_channelList_itemSelectionChanged();
    void on_channelList_currentRowChanged(int currentRow);
    void on_verticalScrollBar_valueChanged(int value);

  private:
    static struct ActionEntry
    {
        typedef void (BCI2000Viewer::*Action)();
        typedef bool (BCI2000Viewer::*StateGetter)() const;
        Action action;
        StateGetter enabled, checked;
        const char *object, *group, *title;
    } sActions[];
    void SetupActions();
  private slots:
    void ExecuteAction();
    void UpdateActions();

  public:
    // User actions
    // Standard actions
    void FileOpen();
    bool FileOpen_Enabled() const;
    void FileOpenInfo();
    bool FileOpenInfo_Enabled() const;
    void FileClose();
    bool FileClose_Enabled() const;
    void FileExport();
    bool FileExport_Enabled() const;
    void FileQuit();
    bool FileQuit_Enabled() const;
    void EditCopy();
    bool EditCopy_Enabled() const;
    void HelpOpenHelp();
    void HelpAbout();
    // Temporal movement
    void ToFirstSample();
    void ToLastSample();
    void ToPrevBlock();
    void ToNextBlock();
    void ToPrevPage();
    void ToNextPage();
    bool GoBack_Enabled() const;
    bool GoForward_Enabled() const;
    // Temporal resolution
    void SampleZoomIn();
    bool SampleZoomIn_Enabled() const;
    void SampleZoomOut();
    bool SampleZoomOut_Enabled() const;
    void ResetSampleZoom();
    bool ResetSampleZoom_Enabled() const;
    // Number of displayed channels
    void FewerChannels();
    bool FewerChannels_Enabled() const;
    void MoreChannels();
    bool MoreChannels_Enabled() const;
    // Channel scrolling
    void ChannelUp();
    void ChannelDown();
    void ChannelPageNext();
    void ChannelPagePrev();
    void ChannelPageFirst();
    void ChannelPageLast();
    bool ChannelUp_Enabled();
    bool ChannelDown_Enabled();
    // Signal resolution
    void RescaleSignal(char op, double minValue, double maxValue);
    void EnlargeSignal();
    void ReduceSignal();
    bool ChangeResolution_Enabled() const;
    void ResetSignalScale();
    bool ResetSignalScale_Enabled() const;
    // Display attributes
    void ToggleColorSelector();
    bool ToggleColorSelector_Enabled() const;
    bool ToggleColorSelector_Checked() const;
    void ColorSelectorClosed();
    void ApplyColorSelection(const QColor&);
    void ApplyColorPattern(const QString&);
    void ApplyColorReset();

    void ColorThemeAuto();
    bool ColorThemeAuto_Checked() const;
    void ColorThemeLight();
    bool ColorThemeLight_Checked() const;
    void ColorThemeDark();
    bool ColorThemeDark_Checked() const;
    void ApplyColorTheme();

    void ToggleGrid();
    bool ToggleGrid_Checked() const;
    bool ToggleGrid_Enabled() const;
    void ToggleBaselines();
    bool ToggleBaselines_Checked() const;
    bool ToggleBaselines_Enabled() const;
    void ToggleUnit();
    bool ToggleUnit_Checked() const;
    bool ToggleUnit_Enabled() const;
    // Filters
    void HPOff();
    bool HPOff_Checked() const;
    void HPRemoveMean();
    bool HPRemoveMean_Checked() const;
    void HP1Hz();
    bool HP1Hz_Checked() const;
    void HP5Hz();
    bool HP5Hz_Checked() const;
    void HPCustom();
    bool HPCustom_Checked() const;
    void LPOff();
    bool LPOff_Checked() const;
    void LP30Hz();
    bool LP30Hz_Checked() const;
    void LP40Hz();
    bool LP40Hz_Checked() const;
    void LP70Hz();
    bool LP70Hz_Checked() const;
    void LPCustom();
    bool LPCustom_Checked() const;
    void NotchOff();
    void Notch50Hz();
    void Notch60Hz();
    bool Filter_Enabled() const;
    bool GetCustomFilter(const char* name, const char* abbrev, double (DisplayFilter::*getter)() const, DisplayFilter& (DisplayFilter::*setter)(double));
    // Channel List
    void ShowSelectedChannels();
    void HideSelectedChannels();
    bool ChannelsSelected() const;
    void HelpOnState();
    bool StateSelected() const;
    // Notes
    void ToggleNotes();
    bool ToggleNotes_Enabled() const;
    bool ToggleNotes_Checked() const;
    // States/Channels
    void ToggleStatesChannels();
    bool ToggleStatesChannels_Enabled() const;
    bool ToggleStatesChannels_Checked() const;
    // Scrolling
    void ToggleFastScrolling();
    bool ToggleFastScrolling_Checked() const;
    // Decimation
    void ToggleAutoDecimation();
    bool ToggleAutoDecimation_Checked() const;

    int64_t NumSamples() const;
    int64_t NumDisplaySamples() const;
    void SetSamplePos(int64_t sampleIndex, bool propagate = true);
    void ScrollSamplePosBy(int64_t sampleIndexDiff, bool propagate = true);
  public slots:
    void ScrollToSamplePos(int64_t sampleIndex, bool propagate = true);
    void SetNumDisplaySamples(int sampleCount, bool propagate = true);

  public slots:
    void Play();
    void Pause();
    void SetVolume(float);
    void SetPlaybackRatio(float);

  public:
    bool Playing() const;
  private slots:
    void PlaybackTimer();

  private:
    // Internal functions
    void DoFileOpen(const QString &name);
    static std::string GetSecondaryFile(const std::string& primaryFile);
    void FillChannelList();

    const char* SettingsPrefix() const;
    void SaveSettings() const;
    void ReadSettings();
    void RestoreDefaultGeometry();

    void UpdateSamplePos(bool inPropagate);
    void UpdateTimeField();
    void UpdateChannelLabels();
    void UpdateVerticalScroller();
    GenericSignal ConstructDisplaySignal(int64_t samplePos, int64_t length);
    void FilterChanged();
    bool FilterActive();

    void PopulateToolbars();
    void LoadActionIcons();
    QPixmap LoadIcon(const bci::Resource&, bool adapt = true);
    QPixmap LoadIcon(const QIcon&, bool adapt = true);

    void OpenNotes();
    void UpdateNotes();
    void CloseNotes();
  private slots:
    void SyncWithNote(int);

  private:
    void CreateAVObjects();
    void UpdateAVObjects();
    void DestroyAVObjects();
    void SyncStatevectorForAudio();
  private slots:
    void BringPlaybackControlToFront();
    void BringVideoWindowToFront();

  private:
    void CreateSecondaryViewer();
    void UpdatePairedViewerPos();
    void UpdatePairedViewerSampleZoom();
    void DestroySecondaryViewer();
    void IOThread();
  private slots:
    void OnIOThreadTimer();

  private:
    int64_t LastSample() const;

    void SetChannelSelected(int, bool);
    void ExtendChannelSelection(int);
    bool ToggleChannelSelected(int);
    void UpdateChannelSelection();
    bool IsChannelVisible(int) const;
    void SelectAllChannels();
    void ClearChannelSelection();
    bool AnyChannelsSelected() const;
    std::vector<bool> ExtendedChannelSelection() const;

    std::pair<int, int> YRangeToChannels(float y0, float y1) const;


  private:
    static const RGBColor cAxisColor;
    static const RGBColor cChannelColorsDefault[];
    static const QColor cHeadingColor;
    static const int cAutoDecimationMaxLinesPerPixel;

    enum { auto_, light_, dark_ };
    int mColorTheme = dark_;
    QToolButton* mpColorThemeButton = nullptr,
                *mpHPButton = nullptr,
                *mpLPButton = nullptr,
                *mpNotchButton = nullptr;

    float mMouseY = 0, mDMouseY = 0;
    int mSelectionAnchor = -1;
    std::vector<bool> mChannelsSelected;

    class ExtendedColorSelector* mpColorSelector = nullptr;
    ColorList mOriginalColors;

    BCI2000FileReader mFile;
    int64_t mSamplePos;
    int mNumSignalChannels;
    bool mPositionEdited;
    float mSamplingRate;
    DisplayFilter mFilter;
    bool mRemoveMean;
    std::string mFileName;

    int mNumSamplesDefault = 0;
    double mMinValueDefault = 0,
           mMaxValueDefault = 0;

    NotesWidget* mpNotesWidget;
    StateRef* mpNotesState;

    AudioSinkRef mpAudioSink;

    PlaybackControl *mpPlaybackControl;

    struct VideoLink
    {
        StateRef state;
        VideoWidget *widget;
    };
    std::vector<VideoLink> mVideoWidgets;

    class FrameControlledAudioPlayer : public TimeStretchAudioPlayer
    {
    public:
        FrameControlledAudioPlayer(AudioSinkRef, StateRef, int64_t frameOffset);
        void SyncFramePosition();
        void AdjustPlaybackTempo(double playbackRatio);
    private:
        StateRef mState;
        int64_t mFrameOffset = 0;
        // position difference scaling constant
        // chosen to be on the order of the inverse of 10 - 20 times a typical audio block size
        constexpr static double mAlpha = 1e-4;
    };
    std::vector <std::unique_ptr<FrameControlledAudioPlayer>> mAudioPlayers;

    QTimer mPlaybackTimer;
    float mPlaybackRatio;
    int64_t mLastTimerStamp;
    int64_t mPrevReadSignalDuration;

    ChildProcess* mpSecondaryViewer;
    std::thread mSecondaryViewerThread;
    bool mIsPrimary, mIsSecondary;
};

#endif // BCI2000VIEWER_H
