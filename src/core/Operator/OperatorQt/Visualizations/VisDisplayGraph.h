////////////////////////////////////////////////////////////////////////////////
// $Id: VisDisplayGraph.h 9490 2026-07-21 20:40:26Z mellinger $
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A class for graph type visualization windows.
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
#ifndef VIS_DISPLAY_GRAPH_H
#define VIS_DISPLAY_GRAPH_H

#define AUTO_DECIMATE 0

#include "DecimationFilter.h"
#include "DisplayFilter.h"
#include "MappedProperty.h"
#include "SignalWidget.h"
#include "VisDisplayLayer.h"
#include "Resource.h"

#include <map>
#include <string>

class QMenu;
class QAction;
class QMouseEvent;
class QLabel;
class QToolBar;
class QToolButton;
class QStatusBar;
class QListWidget;
class QListWidgetItem;

class VisDisplaySpectrum;

class VisDisplayGraph : public VisDisplayLayer
{
    Q_OBJECT

  public:
    VisDisplayGraph(const std::string &visID);
    virtual ~VisDisplayGraph();

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
    void BuildStatusBar();
    void ShowStatusBar(bool);
    void BuildGraphContextMenu();
    void BuildChannelListContextMenu();
    void PopulateToolBar();
    void ShowToolBar(bool);
    void LoadActionIcons();

    class FilterMenu
    {
    public:
        FilterMenu();

        void SetMenu(QMenu*);
        QMenu* Menu() { return mpMenu; }

        void SetCurrentEntry(const std::string&);
        const std::string& CurrentEntry() const { return mCurrentEntry; }

        const std::string& LastNonnullEntry() const { return mLastNonnullEntry; }
        std::string NextEntry() const;

     private:
        static bool EntryCmp(const std::string&, const std::string&);

        std::set<std::string, decltype(&EntryCmp)> mFixedEntries;
        std::string mLastNonnullEntry, mCustomEntry, mCurrentEntry;
        QMenu* mpMenu = nullptr;

    } mHPMenu, mLPMenu, mNotchMenu;

    QMenu *mpGraphContextMenu = nullptr, 
          *mpColorThemeMenu = nullptr,
          *mpXLogScaleMenu = nullptr,
          *mpYLogScaleMenu = nullptr;
    QAction *mpActEnlargeSignal = nullptr,
            *mpActReduceSignal = nullptr,
            *mpActResetSignalScale = nullptr,
            *mpActToggleAutoScale = nullptr,
            *mpActFewerSamples = nullptr,
            *mpActMoreSamples = nullptr,
            *mpActResetSampleZoom = nullptr,
            *mpActMoreChannels = nullptr,
            *mpActFewerChannels = nullptr,
            *mpActToggleDisplayMode = nullptr,
            *mpActChannelClickSelect = nullptr,
            *mpActToggleNumericValues = nullptr,
            *mpActToggleColor = nullptr,
            *mpActToggleBaselines = nullptr,
            *mpActToggleGrid = nullptr,
            *mpActToggleChannelLabels = nullptr,
            *mpActToggleColorSelector = nullptr,
            *mpActColorThemeAuto = nullptr,
            *mpActColorThemeLight = nullptr,
            *mpActColorThemeDark = nullptr,
            *mpActColorTheme = nullptr,
            *mpActLargerFont = nullptr,
            *mpActSmallerFont = nullptr,
            *mpActToggleValueUnit = nullptr,
            *mpActShowUnit = nullptr,
            *mpActHideUnit = nullptr, 
            *mpActNewSpectrumDisplay = nullptr,
            *mpActToggleChannelList = nullptr,
            *mpActChannelList_Enable = nullptr,
            *mpActChannelList_Disable = nullptr,
            *mpHPMenuButton = nullptr,
            *mpLPMenuButton = nullptr,
            *mpNotchMenuButton = nullptr,
            *mpSpectrumSeparator = nullptr,
            *mpSpectrumButton = nullptr;
    SignalWidget *mpSignalWidget = nullptr;
    QListWidget *mpChannelList = nullptr;
    QMenu *mpChannelListContextMenu = nullptr;
    QToolBar* mpToolBar = nullptr;
    QToolButton* mpColorThemeButton = nullptr,
                *mpXLogScaleButton = nullptr,
                *mpYLogScaleButton = nullptr;
    QStatusBar *mpStatusBar = nullptr;
    QLabel *mpStatusLabel = nullptr;

  private slots:
    // Menu action slots
    void EnlargeSignal();
    void ReduceSignal();
    void ResetSignalScale();
    void ToggleAutoScale();
    void FewerSamples();
    void MoreSamples();
    void ResetSampleZoom();
    void MoreChannels();
    void FewerChannels();
    void ToggleDisplayMode();
    void ToggleChannelClickSelect();
    void ToggleNumericValues();
    void ToggleValueUnit();
    void ToggleBaselines();
    void ToggleGrid();
    void ToggleChannelLabels();
    void LargerFont();
    void SmallerFont();
    void ToggleChannelList();
    void ToggleColor();

    void ToggleColorSelector();
    void ColorSelectorClosed();
    void ApplyColorSelection(const QColor&);
    void ApplyColorPattern(const QString&);
    void ApplyColorReset();

    void ColorThemeAuto();
    void ColorThemeLight();
    void ColorThemeDark();

    void SetHPOff();
    void SetHP01();
    void SetHP1();
    void SetHP5();
    void SetHPCustom();
    void SetLPOff();
    void SetLP30();
    void SetLP40();
    void SetLP70();
    void SetLPCustom();
    void SetNotchOff();
    void SetNotch50();
    void SetNotch60();

    void SetXScaleLinear();
    void SetXScaleLog10();
    void SetXScaleLog20();
    void SetXScaleLog100();
    void SetXScaleCustom();

    void SetYScaleLinear();
    void SetYScaleLog10();
    void SetYScaleLog20();
    void SetYScaleLog100();
    void SetYScaleCustom();

    void ChannelList_DisableAll();
    void ChannelList_EnableAll();
    void ChannelList_Enable();
    void ChannelList_Disable();
    void OnChannelListItemSelectionChanged();
    void OnChannelListItemChanged(QListWidgetItem*);

    void NewSpectrumDisplay();

private:
    bool EnlargeSignal_Enabled() const;
    bool ReduceSignal_Enabled() const;
    bool ResetSignalScale_Enabled() const;
    bool ToggleAutoScale_Enabled() const;
    bool ToggleAutoScale_Checked() const;

    bool FewerSamples_Enabled() const;
    bool MoreSamples_Enabled() const;
    bool ResetSampleZoom_Enabled() const;
    bool MoreChannels_Enabled() const;
    bool FewerChannels_Enabled() const;

    bool ToggleChannelClickSelect_Checked() const;
    bool ToggleNumericValues_Enabled() const;
    bool ToggleNumericValues_Checked() const;
    bool ToggleBaselines_Enabled() const;
    bool ToggleBaselines_Checked() const;
    bool ToggleGrid_Enabled() const;
    bool ToggleGrid_Checked() const;
    bool ToggleColorSelector_Enabled() const;
    bool ToggleValueUnit_Enabled() const;
    bool ToggleValueUnit_Checked() const;
    bool ToggleChannelLabels_Enabled() const;
    bool ToggleChannelLabels_Checked() const;
    bool ToggleChannelList_Enabled() const;
    bool ToggleChannelList_Checked() const;
    bool SmallerFont_Enabled() const;
    bool ToggleColor_Enabled() const;
    bool ToggleColor_Checked() const;
    bool Spectrum_Enabled() const;

    bool Filter_Enabled() const;
    double FilterUnitToValue(const std::string &inUnit) const;
    double FilterCaptionToValue(const std::string& inCaption) const;
    std::string FilterValueToCaption(double inValue) const;
    std::string NormalizeFilterCaption(const std::string&) const;

    int NominalDisplaySamples() const;
    void SetNominalDisplaySamples(int);
    double NominalUnitsPerSample() const;
    void SetNominalUnitsPerSample(double);

    void SetHP(const std::string &, bool writeToConfig = false);
    void SetLP(const std::string &, bool writeToConfig = false);
    void SetNotch(const std::string &, bool writeToConfig = false);
    void UpdateStatusBar();

    void SetXScale(float scale, bool writeToConfig = false);
    void SetYScale(float scale, bool writeToConfig = false);
    QAction* SyncLogScaleMenu(float, QMenu*);

    void BuildChannelList();
    void UpdateChannelList(const LabelList&);
    int NumVisibleChannelGroups() const;

    bool ChannelList_Enabled() const;

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

    void ApplyUserScaling();
    void SetUserScalingForChannel(int ch, int value);
    int GetUserScalingForChannel(int ch) const;
    void ApplyAutoScale(const PhysicalUnit&);

    void SaveValueUnitsVisible() const;
    void ApplyUserZoom();
    void ApplyColorTheme();

    void CreateSpectrumDisplay(const VisID&);
    void UpdateSpectrumDisplay(const VisID&);
    void RestoreSpectrumDisplays();
    void UpdateSpectrumDisplays();
    void SaveDependants();

  protected:
    void OnDependantCreated(const VisID&) override;
    void OnDependantDestroyed(const VisID&) override;

  private:
    template<class T> void MappedPropertyToContainer(const MappedProperty<T>&, std::vector<T>&) const;
    template<class T> void ContainerToMappedProperty(const std::vector<T>&, MappedProperty<T>&) const;

  private:
    int mNumChannels = 0, mSignalElements = 0, mConfigDisplaySamples = 0;
    double mUserScaling = 0, mUserZoom = 0;
    std::vector<double> mChannelUserScaling;
    double mMinValue = 0, mMaxValue = 0;
    std::vector<double> mMinValues, mMaxValues;
    std::string mAutoScaleTime;
    bool mSignalIsStream = false;
#if AUTO_DECIMATE
    GenericSignal mDecimatedSignal;
#endif
    GenericSignal mFilteredSignal;
    DecimationFilter mDecimationFilter;
    DisplayFilter mDisplayFilter;
    bool mActivateFilters = true;

    bool mAutoScale = false, mFixedScale = false, mFixedSampleCount = false;
    float mElementGain = 0;
    class ExtendedColorSelector* mpColorSelector = nullptr;
    ColorList mOriginalColors;

    std::map<std::string, int> mChannelMap;
    MappedProperty<bool> mChannelsVisible;
    std::vector<bool> mChannelsSelected;
    bool mChannelClickSelect = true;
    int mSelectionAnchor = -1;
    float mMouseY = 0, mDMouseY = 0;
    enum { auto_, light_, dark_ };
    int mColorTheme = dark_;

  protected:
    // Qt event handlers
    void hideEvent(QHideEvent*) override;
    void moveEvent(QMoveEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void changeEvent(QEvent *) override;
};

#endif // VIS_DISPLAY_GRAPH_H
