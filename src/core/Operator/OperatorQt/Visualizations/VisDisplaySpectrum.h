////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A class for spectral analysis visualizations.
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
#ifndef VIS_DISPLAY_SPECTRUM_H
#define VIS_DISPLAY_SPECTRUM_H

#include "VisDisplayLayer.h"
#include "DecimationFilter.h"
#include "DisplayFilter.h"
#include "ContinuousDft.h"
#include "MappedProperty.h"
#include "SignalWidget.h"
#include "Resource.h"

#include <map>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

class QMenu;
class QAction;
class QLabel;
class QToolBar;
class QToolButton;
class QListWidget;
class QListWidgetItem;

class VisDisplaySpectrum : public VisDisplayLayer
{
    Q_OBJECT

  public:
    VisDisplaySpectrum(const std::string &visID);
    virtual ~VisDisplaySpectrum();

  protected:
    void OnSignal(const GenericSignal &) override;
    void OnSetConfig(ConfigSettings &) override;
    void OnContextMenu(const QPoint &) override;

    // User interaction.
  private:
    void OnGraphContextMenu(const QPoint&);
    void UpdateGraphActions();
    void OnChannelListContextMenu(const QPoint&);
    void UpdateChannelListActions();
    void BuildGraphContextMenu();
    void BuildChannelListContextMenu();
    void PopulateToolBar();
    void ShowToolBar(bool);
    void LoadActionIcons();

    QMenu *mpGraphContextMenu = nullptr,
          *mpColorThemeMenu = nullptr,
          *mpValueLogScaleMenu = nullptr,
          *mpFrequencyScaleMenu = nullptr;
    QAction *mpActEnlargeSignal = nullptr,
            *mpActReduceSignal = nullptr,
            *mpActResetSignalScale = nullptr,
            *mpActToggleAutoScale = nullptr,
            *mpActFewerSamples = nullptr,
            *mpActMoreSamples = nullptr,
            *mpActResetSampleZoom = nullptr,
            *mpActFrequencyScaleLinear = nullptr,
            *mpActFrequencyScaleConstantQ = nullptr,
            *mpActMoreFrequencies = nullptr,
            *mpActFewerFrequencies = nullptr,
            *mpActToggleNumericValues = nullptr,
            *mpActToggleColor = nullptr,
            *mpActColorThemeAuto = nullptr,
            *mpActColorThemeLight = nullptr,
            *mpActColorThemeDark = nullptr,
            *mpActColorTheme = nullptr,
            *mpActLargerFont = nullptr,
            *mpActSmallerFont = nullptr,
            *mpActToggleValueUnit = nullptr,
            *mpActShowUnit = nullptr,
            *mpActHideUnit = nullptr, 
            *mpActToggleChannelList = nullptr,
            *mpActChannelList_Activate = nullptr,
            *mpActChannelList_Deactivate = nullptr;
    SignalWidget *mpSignalWidget = nullptr;
    QListWidget *mpChannelList = nullptr;
    QMenu *mpChannelListContextMenu = nullptr;
    QToolBar* mpToolBar = nullptr;
    QToolButton* mpColorThemeButton = nullptr,
                *mpXLogScaleButton = nullptr,
                *mpValueLogScaleButton = nullptr,
                *mpFrequencyScaleButton = nullptr;

  private slots:
    // Menu action slots
    void EnlargeSignal();
    void ReduceSignal();
    void ResetSignalScale();
    void ToggleAutoScale();

    void FewerSamples();
    void MoreSamples();
    void ResetSampleZoom();

    void FewerFrequencies();
    void MoreFrequencies();

    void ToggleNumericValues();
    void ToggleValueUnit();
    void LargerFont();
    void SmallerFont();
    void ToggleChannelList();
    void ToggleColor();

    void ColorThemeAuto();
    void ColorThemeLight();
    void ColorThemeDark();

    void SetValueScaleLinear();
    void SetValueScaleLog10();
    void SetValueScaleLog20();
    void SetValueScaleLog100();
    void SetValueScaleCustom();

    void SetDftUniform();
    void SetDftConstantQ();
    void InitializeDft();
    LabelList BuildFrequencyLabels() const;
    void InitializeDftConfig();
    void SaveDftConfig();
    static void OnDftData(const GenericSignal&, void*);

    void ChannelList_DeactivateAll();
    void ChannelList_ActivateAll();
    void ChannelList_Activate();
    void ChannelList_Deactivate();
    void OnChannelListItemSelectionChanged();
    void OnChannelListItemChanged(QListWidgetItem*);

private:
    bool EnlargeSignal_Enabled() const;
    bool ReduceSignal_Enabled() const;
    bool ResetSignalScale_Enabled() const;
    bool ToggleAutoScale_Checked() const;
    bool FewerSamples_Enabled() const;
    bool MoreSamples_Enabled() const;
    bool ResetSampleZoom_Enabled() const;
    bool FewerFrequencies_Enabled() const;
    bool MoreFrequencies_Enabled() const;
    bool ToggleNumericValues_Checked() const;
    bool ToggleValueUnit_Checked() const;
    bool ToggleChannelList_Enabled() const;
    bool ToggleChannelList_Checked() const;
    bool SmallerFont_Enabled() const;
    bool ToggleColor_Checked() const;
    double UnitToValue(const std::string &inUnit) const;

    int DisplaySamples() const;
    void SetDisplaySamples(int);
    double UnitsPerSample() const;
    void SetUnitsPerSample(double);

    void SetValueScale(float scale, bool writeToConfig = false);
    QAction* SyncLogScaleMenu(float, QMenu*);

    void BuildChannelList();
    void UpdateChannelList(const LabelList&);

    bool ChannelList_Enabled() const;

    void SetChannelSelected(int, bool);
    bool ToggleChannelSelected(int);
    void UpdateChannelSelection();
    void SelectAllChannels();
    void ClearChannelSelection();

    void ApplyUserScaling();

    void ApplyUserZoom();
    void ApplyColorTheme();

    void InitializeAutoScale();
    void ObserveAutoScale(const GenericSignal&);
    void ApplyAutoScale();

    void ProcessingThreadFunc();

  private:
    int mNumChannels = 0, mSignalElements = 0, mConfigDisplaySamples = 0;
    double mUserScaling = 0, mUserZoom = 0;
    double mMinValue = 0, mMaxValue = 0;
    bool mFixedSampleCount = 0;
    double mInputSamplingRate = 0;
    int mSpectrumSize = 0;

    struct {
        bool enabled = false;
        float decayFactor = 0;
        float count = 0, sum = 0;
    } mAutoScaleFilter;

    std::map<std::string, int> mChannelMap;
    MappedProperty<bool> mChannelsVisible;
    std::vector<bool> mChannelsSelected;

    enum { auto_, light_, dark_ };
    int mColorTheme = dark_;

    DisplayFilter mDecimationLp;
    DecimationFilter mDecimationFilter;
    GenericSignal mLpFilteredSignal, mDecimatedSignal;

    typedef float ContinuousDftNumericType;
    ContinuousDft::SignalObserver<ContinuousDftNumericType> mDftObserver;
    enum { linear, constantQ };
    int mDftType = constantQ;
    int mDftDecimation = 1;
    int mDftSpectraDecimation = 1;
    int mDftNumBins = 0;
    ContinuousDft::Config mDftConfig;
    std::mutex mDftMutex;

    std::queue<GenericSignal> mSignalQueue;
    std::atomic<bool> mTerminateProcessingThread;
    std::mutex mSignalQueueMutex;
    std::condition_variable mSignalQueueCondition;
    std::thread mProcessingThread;

  protected:
    // Qt event handlers
    void hideEvent(QHideEvent*) override;
    void moveEvent(QMoveEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void wheelEvent(QWheelEvent*) override;
    void changeEvent(QEvent *) override;
};

#endif // VIS_DISPLAY_SPECTRUM_H
