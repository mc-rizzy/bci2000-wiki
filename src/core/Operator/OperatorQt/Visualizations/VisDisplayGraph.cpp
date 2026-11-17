////////////////////////////////////////////////////////////////////////////////
// $Id: VisDisplayGraph.cpp 9497 2026-07-23 14:57:27Z mellinger $
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
#include "VisDisplayGraph.h"

#include "ExtendedColorSelector.h"
#include "MappedProperty.h"
#include "Resources.h"
#include "ToolButtonAction.h"

#include <limits>
#include <cmath>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QToolBar>
#include <QStatusBar>
#include <QStyle>
#include <QScreen>
#include <QListWidget>
#include <QTimer>

#define OFF "off"

#define DISABLE_SPECTRUM 0

static const int kMaxUserScaling = 32;  // The maximum number of scaling steps a user can take from the default.
static const int kStatusBarHeight = 12; // The height of the status bar, in pixels.

static const std::string kAutoScaleTime = "50ms";
static const double kAutoScaleFactor = 1.3; // additional margin factor
static const int kAutoDecimationMaxLinesPerPixel = 5;

static const RGBColor sInitialCustomColors[] =
{
    RGBColor::White, RGBColor::LtGray,
    RGBColor::DkGray, RGBColor::Black,

    RGBColor::Red, RGBColor::Yellow,
    RGBColor::Orange, RGBColor::Purple,
    RGBColor::Fuchsia, RGBColor::Lime,
    RGBColor::Green, RGBColor::Maroon,
    RGBColor::Blue, RGBColor::Aqua,
    RGBColor::Olive, RGBColor::Navy,

    ColorList::End
};

VisDisplayGraph::VisDisplayGraph(const std::string &inSourceID)
    : VisDisplayLayer(inSourceID)
{
    QVBoxLayout* pVLayout = new QVBoxLayout;
    pVLayout->setContentsMargins(0, 0, 0, 0);
    pVLayout->setSpacing(0);
    QHBoxLayout* pHLayout = new QHBoxLayout;
    pHLayout->setContentsMargins(0, 0, 0, 0);
    pHLayout->setSpacing(0);
    mpToolBar = new QToolBar;
    mpToolBar->setFloatable(false);
    mpToolBar->setOrientation(Qt::Horizontal);
    pVLayout->addWidget(mpToolBar);
    pVLayout->addLayout(pHLayout);
    mpSignalWidget = new SignalWidget(this);
    mpSignalWidget->Display().SetAutoDecimation(kAutoDecimationMaxLinesPerPixel);
    pHLayout->addWidget(mpSignalWidget);
    setFocusProxy(mpSignalWidget);
    BuildStatusBar();
    pVLayout->addWidget(mpStatusBar);
    UpdateStatusBar();
    BuildGraphContextMenu();
    BuildChannelList();
    BuildChannelListContextMenu();
    PopulateToolBar();
    pHLayout->addWidget(mpChannelList);
    setLayout(pVLayout);

    QTimer::singleShot(0, this, &VisDisplayGraph::RestoreSpectrumDisplays);
}

VisDisplayGraph::~VisDisplayGraph()
{
    delete mpGraphContextMenu;
    delete mpChannelListContextMenu;
}

void VisDisplayGraph::OnSetConfig(ConfigSettings &inConfig)
{
    VisDisplayBase::OnSetConfig(inConfig);

    SignalDisplay& display = mpSignalWidget->Display();

    double minValue, maxValue;
    if (inConfig.Get(CfgID::MinValue, minValue))
        mMinValue = minValue;
    if (inConfig.Get(CfgID::MaxValue, maxValue))
        mMaxValue = maxValue;

    ValueList<double> minValues, maxValues;
    if (inConfig.Get(CfgID::MinValues, minValues))
        mMinValues = minValues;
    if (inConfig.Get(CfgID::MaxValues, maxValues))
        mMaxValues = maxValues;

    double userScaling = 0;
    if (inConfig.Get(CfgID::UserScaling, userScaling))
        mUserScaling = userScaling;

    MappedProperty<double> channelUserScaling;
    if (inConfig.Get(CfgID::ChannelUserScaling, channelUserScaling))
        MappedPropertyToContainer(channelUserScaling, mChannelUserScaling);

    ApplyUserScaling();

    inConfig.Get(CfgID::FixedScale, mFixedScale);

    inConfig.Get(CfgID::FixedSampleCount, mFixedSampleCount);

    int numSamples = NominalDisplaySamples();
    if (inConfig.Get(CfgID::NumSamples, numSamples))
    {
        mConfigDisplaySamples = numSamples;
        SetNominalDisplaySamples(numSamples);
    }

    double userZoom = 0;
    if (!mFixedSampleCount)
    {
        if (inConfig.Get(CfgID::UserSampleZoom, userZoom))
        {
            mUserZoom = userZoom;
            ApplyUserZoom();
        }
    }

    int channelGroupSize = display.ChannelGroupSize();
    if (inConfig.Get(CfgID::ChannelGroupSize, channelGroupSize))
    {
        if (channelGroupSize < 1)
            channelGroupSize = std::numeric_limits<int>::max();
        display.SetChannelGroupSize(channelGroupSize);
    }
    int graphType = display.DisplayMode();
    if (inConfig.Get(CfgID::GraphType, graphType))
        switch (graphType)
        {
        case CfgID::Polyline:
            display.SetDisplayMode(SignalDisplay::polyline);
            break;
        case CfgID::Field2d:
            display.SetDisplayMode(SignalDisplay::field2d);
            break;
        }

    bool channelClickSelect = mChannelClickSelect;
    if (inConfig.Get(CfgID::ClickingTraceSelectsChannel, channelClickSelect))
        mChannelClickSelect = channelClickSelect;

    bool showNumericValues = display.NumericValuesVisible();
    if (inConfig.Get(CfgID::ShowNumericValues, showNumericValues))
        display.SetNumericValuesVisible(showNumericValues);

    bool showBaselines = display.BaselinesVisible();
    if (inConfig.Get(CfgID::ShowBaselines, showBaselines))
        display.SetBaselinesVisible(showBaselines);

    bool showGrid = display.GridVisible();
    if (inConfig.Get(CfgID::ShowGrid, showGrid))
        display.SetGridVisible(showGrid);

    int colorTheme = mColorTheme;
    if (inConfig.Get(CfgID::ColorTheme, colorTheme))
        mColorTheme = colorTheme;

    bool inverted = display.LightMode();
    if (inConfig.Get(CfgID::InvertedDisplay, inverted))
        mColorTheme = inverted ? light_ : dark_;

    ApplyColorTheme();

    int fontSize = display.FontSize();
    if (inConfig.Get(CfgID::FontSize, fontSize))
        display.SetFontSize(fontSize);

    // Channel colors must be stored as a ColorList (rather than a MappedProperty<RGBColor>)
    // for backward compatibility.
    ColorList channelColors = display.ChannelColors();
    if (inConfig.Get(CfgID::ChannelColors, channelColors))
        display.SetChannelColors(channelColors);

    std::string channelStyles = display.ChannelStyles();
    if (inConfig.Get(CfgID::ChannelStyles, channelStyles))
        display.SetChannelStyles(channelStyles);

    std::string valueColors;
    if (inConfig.Get(CfgID::ValueColors, valueColors))
        display.SetValueColors(valueColors);

    std::string unit;
    if (inConfig.Get(CfgID::SampleUnit, unit))
    {
        double oldSampleUnit = NominalUnitsPerSample() * FilterUnitToValue(display.SampleUnit());
        PhysicalUnit u;
        u.SetGainWithSymbol(unit);
        float unitsPerSample = u.Gain();
        std::string sampleUnit = u.Symbol();
        SetNominalUnitsPerSample(unitsPerSample);
        display.SetSampleUnit(u.Symbol());
        if (oldSampleUnit != unitsPerSample * FilterUnitToValue(sampleUnit))
        {
            mDisplayFilter.HPCorner(0);
            mDisplayFilter.LPCorner(0);
            mDisplayFilter.NotchCenter(0);
        }
        mElementGain = unitsPerSample * FilterUnitToValue(sampleUnit);
    }
    float sampleOffset;
    if (inConfig.Get(CfgID::SampleOffset, sampleOffset))
        display.SetSampleOffset(-sampleOffset);

    if (mFixedScale)
    {
        mAutoScale = false;
        mAutoScaleTime = OFF;
    }
    else
    {
        std::string autoScale;
        mAutoScale = inConfig.Get(CfgID::AutoScale, autoScale);
        if (autoScale == OFF)
        {
            mAutoScale = false;
        }
        else
        {
            if (autoScale.empty() || autoScale.back() != 's')
                autoScale = kAutoScaleTime;
            mAutoScale = true;
        }
        mAutoScaleTime = autoScale;
    }
    if (inConfig.Get(CfgID::ChannelUnit, unit))
        display.SetChannelUnit(unit);

    if (inConfig.Get(CfgID::ValueUnit, unit))
        display.SetValueUnit(unit);

    ValueList<std::string> units = display.ValueUnits();
    if (inConfig.Get(CfgID::ValueUnits, units))
        display.SetValueUnits(units);

    bool showValueUnit = display.ValueUnitVisible();
    if (inConfig.Get(CfgID::ShowValueUnit, showValueUnit))
        display.SetValueUnitVisible(showValueUnit);

    bool xAxisLabels = display.XTicksVisible();
    if (inConfig.Get(CfgID::ShowXTicks, xAxisLabels))
        display.SetXTicksVisible(xAxisLabels);

    bool yAxisLabels = display.YTicksVisible();
    if (inConfig.Get(CfgID::ShowYTicks, yAxisLabels))
      display.SetYTicksVisible(yAxisLabels);

    LabelList labels = display.XAxisMarkers();
    if (inConfig.Get(CfgID::XAxisMarkers, labels))
        display.SetXAxisMarkers(labels);

    labels = display.YAxisMarkers();
    if (inConfig.Get(CfgID::YAxisMarkers, labels))
        display.SetYAxisMarkers(labels);

    inConfig.Get(CfgID::ChannelsVisible, mChannelsVisible);

    labels = display.ChannelLabels();
    if (inConfig.Get(CfgID::ChannelLabels, labels))
    {
        display.SetChannelLabels(labels);
        display.SetChannelLabelsVisible(!labels.empty());
        UpdateChannelList(labels);
    }

    MappedProperty<bool> valueUnitsVisibleByName;
    if (inConfig.Get(CfgID::ValueUnitsVisible, valueUnitsVisibleByName))
    {
        std::vector<bool> valueUnitsVisible = display.ValueUnitsVisible();
        MappedPropertyToContainer(valueUnitsVisibleByName, valueUnitsVisible);
        display.SetValueUnitsVisible(valueUnitsVisible);
    }

    bool activateFilters = true;
    if (inConfig.Get(CfgID::ActivateFilters, activateFilters))
        mActivateFilters = activateFilters;

    std::string hpFilter = OFF;
    if (activateFilters && inConfig.Get(CfgID::HPFilter, hpFilter))
        SetHP(hpFilter);

    std::string lpFilter = OFF;
    if (activateFilters && inConfig.Get(CfgID::LPFilter, lpFilter))
        SetLP(lpFilter);

    std::string notchFilter = OFF;
    if (activateFilters && inConfig.Get(CfgID::NotchFilter, notchFilter))
        SetNotch(notchFilter);

    float xAxisLogScale = display.SampleLogScale();
    if (inConfig.Get(CfgID::XAxisLogScale, xAxisLogScale))
        SetXScale(xAxisLogScale);

    float yAxisLogScale = display.ValueLogScale();
    if (inConfig.Get(CfgID::YAxisLogScale, yAxisLogScale))
        SetYScale(yAxisLogScale);

    bool showStatusBar = true;
    inConfig.Get(CfgID::ShowStatusBar, showStatusBar);
    ShowStatusBar(showStatusBar && Filter_Enabled());

    bool showToolBar = true;
    inConfig.Get(CfgID::ShowToolBar, showToolBar);
    ShowToolBar(showToolBar);

    bool showChannelList = (graphType != CfgID::Polyline);
    inConfig.Get(CfgID::ShowChannelList, showChannelList);
    if (mpChannelList)
        mpChannelList->setVisible(showChannelList);

    bool signalIsStream = false;
    inConfig.Get(CfgID::SignalIsStream, signalIsStream);
    mSignalIsStream = signalIsStream;

    // Sanity check.
    if (display.MinValue() == display.MaxValue())
        display.SetMaxValue(display.MinValue() + 1);

    UpdateGraphActions();
    UpdateChannelListActions();
    UpdateSpectrumDisplays();
}

void VisDisplayGraph::RestoreSpectrumDisplays()
{
    ValueList<std::string> dependants;
    Visconfigs()[mVisID].Get(CfgID::Dependants, dependants);
    for (const auto& dependant : dependants) {
        CreateSpectrumDisplay(dependant);
    }
}

void VisDisplayGraph::UpdateSpectrumDisplays()
{
    for (const auto& visID : Dependants()) {
        UpdateSpectrumDisplay(visID);
    }
}

void VisDisplayGraph::ShowStatusBar(bool inVisible)
{
    mpStatusBar->setVisible(inVisible);
}

void VisDisplayGraph::OnSignal(const GenericSignal &s)
{
    if (s.Channels() < 1 || s.Elements() < 1)
        return;

    SignalDisplay& display = mpSignalWidget->Display();

    int curChannels = mNumChannels, curSamples = display.NumSamples();

    // Apply the visualization filter.
    mFilteredSignal.SetProperties(s.Properties());
    mDisplayFilter.Process(s, mFilteredSignal);
    if (mElementGain > 0)
    {
        SignalProperties s = mFilteredSignal.Properties();
        s.ElementUnit().SetSymbol("s").SetGain(mElementGain);
        mFilteredSignal.SetProperties(s);
    }

    mNumChannels = mFilteredSignal.Channels();

#if AUTO_DECIMATE
    // Apply the decimation filter.
    mDecimationFilter.Process(mFilteredSignal, mDecimatedSignal);

    // Add the signal to the display.
    display.WrapForward(mDecimatedSignal);

    for (auto visID : Dependants())
        VisDisplayBase::HandleSignal(visID.ToLayer(), mDecimatedSignal);

    ApplyAutoScale(mDecimatedSignal.Properties().ElementUnit());
#else
    display.WrapForward(mFilteredSignal);

    for (auto visID : Dependants())
        VisDisplayBase::HandleSignal(visID.ToLayer(), mFilteredSignal);

    ApplyAutoScale(mFilteredSignal.Properties().ElementUnit());
#endif

    mSignalElements = s.Elements();

    if (curChannels != mNumChannels || curSamples != display.NumSamples())
        OnSetConfig(Visconfigs()[mVisID]);
}

void VisDisplayGraph::BuildStatusBar()
{
    mpStatusBar = new QStatusBar(this);
    mpStatusBar->setAutoFillBackground(true);
    mpStatusLabel = new QLabel(mpStatusBar);
    mpStatusBar->addPermanentWidget(mpStatusLabel);
}

void VisDisplayGraph::LoadActionIcons()
{
    mpActEnlargeSignal->setIcon(LoadIcon(Resources::zoom_in_v_icon));
    mpActReduceSignal->setIcon(LoadIcon(Resources::zoom_out_v_icon));
    mpActResetSignalScale->setIcon(LoadIcon(Resources::zoom_reset_v_icon));
    mpActToggleAutoScale->setIcon(LoadIcon(Resources::autoscale_icon));

    mpActFewerSamples->setIcon(LoadIcon(Resources::zoom_in_h_icon));
    mpActMoreSamples->setIcon(LoadIcon(Resources::zoom_out_h_icon));
    mpActResetSampleZoom->setIcon(LoadIcon(Resources::zoom_reset_h_icon));

    mpActMoreChannels->setIcon(LoadIcon(Resources::plus_icon));
    mpActFewerChannels->setIcon(LoadIcon(Resources::minus_icon));
    mpActToggleGrid->setIcon(LoadIcon(Resources::grid_icon));
    mpActToggleColorSelector->setIcon(LoadIcon(Resources::colorwheel_icon, false));
    if (mpColorThemeButton)
        mpColorThemeButton->setIcon(LoadIcon(Resources::light_dark_icon));
    if (mpXLogScaleButton)
        mpXLogScaleButton->setIcon(LoadIcon(Resources::logarithmic_icon));
    if (mpYLogScaleButton)
        mpYLogScaleButton->setIcon(LoadIcon(Resources::logarithmic_icon));
}

void VisDisplayGraph::BuildGraphContextMenu()
{
    mpGraphContextMenu = new QMenu;
    mpActEnlargeSignal = mpGraphContextMenu->addAction("Enlarge Signal", this, &VisDisplayGraph::EnlargeSignal);
    mpActReduceSignal = mpGraphContextMenu->addAction("Reduce Signal", this, &VisDisplayGraph::ReduceSignal);
    mpActResetSignalScale = mpGraphContextMenu->addAction("Reset Signal Scale", this, &VisDisplayGraph::ResetSignalScale);
    mpActToggleAutoScale = mpGraphContextMenu->addAction("Auto Scale", this, &VisDisplayGraph::ToggleAutoScale);
    mpActToggleAutoScale->setCheckable(true);

    mpYLogScaleMenu = new QMenu("Y Scale Type", mpGraphContextMenu);
    QActionGroup* pYLogScaleGroup = new QActionGroup(mpYLogScaleMenu);
    QAction* pAct = mpYLogScaleMenu->addAction("linear", this, SLOT(SetYScaleLinear()));
    pAct->setCheckable(true);
    pYLogScaleGroup->addAction(pAct);
    pAct->setChecked(true);
    pAct = mpYLogScaleMenu->addAction("log10", this, SLOT(SetYScaleLog10()));
    pAct->setCheckable(true);
    pYLogScaleGroup->addAction(pAct);
    pAct = mpYLogScaleMenu->addAction("log20", this, SLOT(SetYScaleLog20()));
    pAct->setCheckable(true);
    pYLogScaleGroup->addAction(pAct);
    pAct = mpYLogScaleMenu->addAction("log100", this, SLOT(SetYScaleLog100()));
    pAct->setCheckable(true);
    pYLogScaleGroup->addAction(pAct);
    mpYLogScaleMenu->addSeparator();
    pAct = mpYLogScaleMenu->addAction("Other...", this, SLOT(SetYScaleCustom()));
    pAct->setCheckable(false);
    mpGraphContextMenu->addMenu(mpYLogScaleMenu);
    mpGraphContextMenu->addSeparator();

    mpActFewerSamples = mpGraphContextMenu->addAction("Fewer Samples", this, &VisDisplayGraph::FewerSamples);
    mpActMoreSamples = mpGraphContextMenu->addAction("More Samples", this, &VisDisplayGraph::MoreSamples);
    mpActResetSampleZoom = mpGraphContextMenu->addAction("Reset Sample Zoom", this, &VisDisplayGraph::ResetSampleZoom);

    mpXLogScaleMenu = new QMenu("X Scale Type", mpGraphContextMenu);
    QActionGroup* pXLogScaleGroup = new QActionGroup(mpXLogScaleMenu);
    pAct = mpXLogScaleMenu->addAction("linear", this, SLOT(SetXScaleLinear()));
    pAct->setCheckable(true);
    pXLogScaleGroup->addAction(pAct);
    pAct->setChecked(true);
    pAct = mpXLogScaleMenu->addAction("log10", this, SLOT(SetXScaleLog10()));
    pAct->setCheckable(true);
    pXLogScaleGroup->addAction(pAct);
    pAct = mpXLogScaleMenu->addAction("log20", this, SLOT(SetXScaleLog20()));
    pAct->setCheckable(true);
    pXLogScaleGroup->addAction(pAct);
    pAct = mpXLogScaleMenu->addAction("log100", this, SLOT(SetXScaleLog100()));
    pAct->setCheckable(true);
    pXLogScaleGroup->addAction(pAct);
    mpXLogScaleMenu->addSeparator();
    pAct = mpXLogScaleMenu->addAction("Other...", this, SLOT(SetXScaleCustom()));
    pAct->setCheckable(false);
    mpGraphContextMenu->addMenu(mpXLogScaleMenu);
    mpGraphContextMenu->addSeparator();

    mpActMoreChannels = mpGraphContextMenu->addAction("More Channels", this, &VisDisplayGraph::MoreChannels);
    mpActFewerChannels = mpGraphContextMenu->addAction("Fewer Channels", this, &VisDisplayGraph::FewerChannels);
    mpGraphContextMenu->addSeparator();

    mpActToggleDisplayMode = mpGraphContextMenu->addAction("Toggle Display Mode", this, SLOT(ToggleDisplayMode()));
    mpActToggleColor = mpGraphContextMenu->addAction("Color Display", this, SLOT(ToggleColor()));
    mpActToggleColor->setCheckable(true);
    
    mpColorThemeMenu = new QMenu("Color Theme");
    QActionGroup* pColorThemeGroup = new QActionGroup(mpColorThemeMenu);
    mpActColorThemeAuto = mpColorThemeMenu->addAction("Auto", this, &VisDisplayGraph::ColorThemeAuto);
    mpActColorThemeAuto->setCheckable(true);
    pColorThemeGroup->addAction(mpActColorThemeAuto);
    mpActColorThemeLight = mpColorThemeMenu->addAction("Light", this, &VisDisplayGraph::ColorThemeLight);
    mpActColorThemeLight->setCheckable(true);
    pColorThemeGroup->addAction(mpActColorThemeLight);
    mpActColorThemeDark = mpColorThemeMenu->addAction("Dark", this, &VisDisplayGraph::ColorThemeDark);
    mpActColorThemeDark->setCheckable(true);
    mpActColorThemeDark->setChecked(true);
    pColorThemeGroup->addAction(mpActColorThemeDark);
    mpGraphContextMenu->addMenu(mpColorThemeMenu);

    mpActToggleColorSelector = mpGraphContextMenu->addAction("Choose Colors...", this, &VisDisplayGraph::ToggleColorSelector);
    mpActToggleColorSelector->setCheckable(true);
    mpActToggleColorSelector->setChecked(false);

    mpGraphContextMenu->addSeparator();

    mpActChannelClickSelect = mpGraphContextMenu->addAction("Clicking Traces Selects Channels", this, &VisDisplayGraph::ToggleChannelClickSelect);
    mpActChannelClickSelect->setCheckable(true);
    mpActChannelClickSelect->setChecked(mChannelClickSelect);

    mpActToggleNumericValues = mpGraphContextMenu->addAction("Show Numeric Values", this, SLOT(ToggleNumericValues()));
    mpActToggleNumericValues->setCheckable(true);
    mpActToggleBaselines = mpGraphContextMenu->addAction("Show Baselines", this, SLOT(ToggleBaselines()));
    mpActToggleBaselines->setCheckable(true);
    mpActToggleGrid = mpGraphContextMenu->addAction("Show Grid", this, SLOT(ToggleGrid()));
    mpActToggleGrid->setCheckable(true);
    mpActToggleChannelLabels = mpGraphContextMenu->addAction("Show Legend", this, SLOT(ToggleChannelLabels()));
    mpActToggleChannelLabels->setCheckable(true);
    mpActToggleValueUnit = mpGraphContextMenu->addAction("Show Value Unit", this, &VisDisplayGraph::ToggleValueUnit);
    mpActToggleValueUnit->setCheckable(true);
    mpActToggleChannelList = mpGraphContextMenu->addAction("Show Channel List", this, SLOT(ToggleChannelList()));
    mpActToggleChannelList->setCheckable(true);
    mpActLargerFont = mpGraphContextMenu->addAction("Larger Font", this, SLOT(LargerFont()));
    mpActSmallerFont = mpGraphContextMenu->addAction("Smaller Font", this, SLOT(SmallerFont()));

    mpGraphContextMenu->addSeparator();

    mpActNewSpectrumDisplay = mpGraphContextMenu->addAction("New Spectrum Display", this, &VisDisplayGraph::NewSpectrumDisplay);

    mpGraphContextMenu->addSeparator();

    QMenu* pMenu = new QMenu("High Pass", mpGraphContextMenu);
    QActionGroup *pHPGroup = new QActionGroup(pMenu);
    pAct = pMenu->addAction(OFF, this, SLOT(SetHPOff()));
    pAct->setCheckable(true);
    pHPGroup->addAction(pAct);
    pAct->setChecked(true);
    pAct = pMenu->addAction("0.1Hz", this, SLOT(SetHP01()));
    pAct->setCheckable(true);
    pHPGroup->addAction(pAct);
    pAct = pMenu->addAction("1Hz", this, SLOT(SetHP1()));
    pAct->setCheckable(true);
    pHPGroup->addAction(pAct);
    pAct = pMenu->addAction("5Hz", this, SLOT(SetHP5()));
    pAct->setCheckable(true);
    pHPGroup->addAction(pAct);
    pMenu->addSeparator();
    pAct = pMenu->addAction("Other...", this, SLOT(SetHPCustom()));
    pAct->setCheckable(false);
    mpGraphContextMenu->addMenu(pMenu);
    mHPMenu.SetMenu(pMenu);

    pMenu = new QMenu("Low Pass", mpGraphContextMenu);
    QActionGroup *pLPGroup = new QActionGroup(pMenu);
    pAct = pMenu->addAction(OFF, this, SLOT(SetLPOff()));
    pAct->setCheckable(true);
    pAct->setChecked(true);
    pLPGroup->addAction(pAct);
    pAct = pMenu->addAction("30Hz", this, SLOT(SetLP30()));
    pAct->setCheckable(true);
    pLPGroup->addAction(pAct);
    pAct = pMenu->addAction("40Hz", this, SLOT(SetLP40()));
    pAct->setCheckable(true);
    pLPGroup->addAction(pAct);
    pAct = pMenu->addAction("70Hz", this, SLOT(SetLP70()));
    pAct->setCheckable(true);
    pLPGroup->addAction(pAct);
    pMenu->addSeparator();
    pAct = pMenu->addAction("Other...", this, SLOT(SetLPCustom()));
    pAct->setCheckable(false);
    mpGraphContextMenu->addMenu(pMenu);
    mLPMenu.SetMenu(pMenu);

    pMenu = new QMenu("Notch", mpGraphContextMenu);
    QActionGroup *pNotchGroup = new QActionGroup(pMenu);
    pAct = pMenu->addAction(OFF, this, SLOT(SetNotchOff()));
    pAct->setCheckable(true);
    pAct->setChecked(true);
    pNotchGroup->addAction(pAct);
    pAct = pMenu->addAction("50Hz", this, SLOT(SetNotch50()));
    pAct->setCheckable(true);
    pNotchGroup->addAction(pAct);
    pAct = pMenu->addAction("60Hz", this, SLOT(SetNotch60()));
    pAct->setCheckable(true);
    pNotchGroup->addAction(pAct);
    mpGraphContextMenu->addMenu(pMenu);
    mNotchMenu.SetMenu(pMenu);
}

void VisDisplayGraph::BuildChannelListContextMenu()
{
    QMenu* pMenu = new QMenu;
    
    pMenu->addAction("Enable all", this, &VisDisplayGraph::ChannelList_EnableAll);
    pMenu->addAction("Disable all", this, &VisDisplayGraph::ChannelList_DisableAll);
    pMenu->addSeparator();

    mpActChannelList_Enable = pMenu->addAction("Enable", this, &VisDisplayGraph::ChannelList_Enable);
    mpActChannelList_Disable = pMenu->addAction("Disable", this, &VisDisplayGraph::ChannelList_Disable);

    mpChannelListContextMenu = pMenu;
}

void VisDisplayGraph::PopulateToolBar()
{
    mpToolBar->clear();
    mpToolBar->addAction(mpActToggleAutoScale);
    mpToolBar->addAction(new ToolButtonAction("Y Scale Type", mpYLogScaleMenu, &mpYLogScaleButton));
    mpToolBar->addAction(mpActEnlargeSignal);
    mpToolBar->addAction(mpActReduceSignal);
    mpToolBar->addAction(mpActResetSignalScale);
    mpToolBar->addSeparator();
    mpToolBar->addAction(new ToolButtonAction("X Scale Type", mpXLogScaleMenu, &mpXLogScaleButton));
    mpToolBar->addAction(mpActMoreSamples);
    mpToolBar->addAction(mpActFewerSamples);
    mpToolBar->addAction(mpActResetSampleZoom);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpActMoreChannels);
    mpToolBar->addAction(mpActFewerChannels);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpActToggleGrid);
    mpToolBar->addAction(mpActToggleColorSelector);
    mpActColorTheme = new ToolButtonAction("Color Theme", mpColorThemeMenu, &mpColorThemeButton);
    mpToolBar->addAction(mpActColorTheme);
    mpToolBar->addSeparator();
    mpHPMenuButton = new ToolButtonAction("HP", mHPMenu.Menu());
    mpToolBar->addAction(mpHPMenuButton);
    mpLPMenuButton = new ToolButtonAction("LP", mLPMenu.Menu());
    mpToolBar->addAction(mpLPMenuButton);
    mpNotchMenuButton = new ToolButtonAction("Notch", mNotchMenu.Menu());
    mpToolBar->addAction(mpNotchMenuButton);

    mpSpectrumSeparator = mpToolBar->addSeparator();
    mpSpectrumButton = mpToolBar->addAction("Spectrum", this, &VisDisplayGraph::NewSpectrumDisplay);

    LoadActionIcons();
}

void VisDisplayGraph::ShowToolBar(bool b)
{
    mpToolBar->setVisible(b);
}

void VisDisplayGraph::OnContextMenu(const QPoint &inP)
{
    if (mpChannelList && mpChannelList->isVisible() && mpChannelList->geometry().contains(inP)) {
        OnChannelListContextMenu(inP);
    }
    else {
        OnGraphContextMenu(inP);
    }
}

void VisDisplayGraph::OnGraphContextMenu(const QPoint &inP)
{
    mpGraphContextMenu->exec(this->mapToGlobal(inP));
}

void VisDisplayGraph::UpdateGraphActions()
{
    mpActEnlargeSignal->setEnabled(EnlargeSignal_Enabled());
    mpActReduceSignal->setEnabled(ReduceSignal_Enabled());
    mpActResetSignalScale->setEnabled(ResetSignalScale_Enabled());
    mpActToggleAutoScale->setEnabled(ToggleAutoScale_Enabled());
    mpActToggleAutoScale->setChecked(ToggleAutoScale_Checked());

    mpActFewerSamples->setEnabled(FewerSamples_Enabled());
    mpActMoreSamples->setEnabled(MoreSamples_Enabled());
    mpActResetSampleZoom->setEnabled(ResetSampleZoom_Enabled());

    mpActFewerChannels->setEnabled(FewerChannels_Enabled());
    mpActMoreChannels->setEnabled(MoreChannels_Enabled());

    mpActChannelClickSelect->setChecked(ToggleChannelClickSelect_Checked());

    mpActToggleNumericValues->setEnabled(ToggleNumericValues_Enabled());
    mpActToggleNumericValues->setChecked(ToggleNumericValues_Checked());

    mpActToggleColor->setEnabled(ToggleColor_Enabled());
    mpActToggleColor->setChecked(ToggleColor_Checked());

    mpActToggleBaselines->setEnabled(ToggleBaselines_Enabled());
    mpActToggleBaselines->setChecked(ToggleBaselines_Checked());

    mpActToggleGrid->setEnabled(ToggleGrid_Enabled());
    mpActToggleGrid->setChecked(ToggleGrid_Checked());

    mpActToggleColorSelector->setEnabled(ToggleColorSelector_Enabled());

    mpActColorThemeAuto->setChecked(mColorTheme == auto_);
    mpActColorThemeLight->setChecked(mColorTheme == light_);
    mpActColorThemeDark->setChecked(mColorTheme == dark_);

    mpActToggleChannelLabels->setEnabled(ToggleChannelLabels_Enabled());
    mpActToggleChannelLabels->setChecked(ToggleChannelLabels_Checked());

    mpActToggleValueUnit->setEnabled(ToggleValueUnit_Enabled());
    mpActToggleValueUnit->setChecked(ToggleValueUnit_Checked());

    mpActToggleChannelList->setEnabled(ToggleChannelList_Enabled());
    mpActToggleChannelList->setChecked(ToggleChannelList_Checked());

    mpActSmallerFont->setEnabled(SmallerFont_Enabled());

    mpActNewSpectrumDisplay->setEnabled(Spectrum_Enabled());
    mpSpectrumButton->setVisible(Spectrum_Enabled());
    mpSpectrumSeparator->setVisible(Spectrum_Enabled());

    mpHPMenuButton->setEnabled(Filter_Enabled());
    mHPMenu.Menu()->setEnabled(Filter_Enabled());
    mpLPMenuButton->setEnabled(Filter_Enabled());
    mLPMenu.Menu()->setEnabled(Filter_Enabled());
    mNotchMenu.Menu()->setEnabled(Filter_Enabled());
    mpNotchMenuButton->setEnabled(Filter_Enabled());
}

void VisDisplayGraph::OnChannelListContextMenu(const QPoint& inP)
{
    UpdateChannelListActions();
    mpChannelListContextMenu->exec(this->mapToGlobal(inP));
}

void VisDisplayGraph::UpdateChannelListActions()
{
    bool enabled = ChannelList_Enabled();
    for (auto pAction : {
        mpActChannelList_Enable,
        mpActChannelList_Disable,
        })
    {
        pAction->setEnabled(enabled);
    }
}

void VisDisplayGraph::ToggleAutoScale()
{
    mAutoScale = !mAutoScale;
    if (mAutoScale)
    {
        mAutoScaleTime = kAutoScaleTime;
    }
    else
    {
        mAutoScaleTime = OFF;
        ApplyUserScaling();
    }
    Visconfigs()[mVisID].Put(CfgID::AutoScale, mAutoScaleTime, UserDefined);
    UpdateGraphActions();
}

bool VisDisplayGraph::ToggleAutoScale_Enabled() const
{
    return !mFixedScale;
}

bool VisDisplayGraph::ToggleAutoScale_Checked() const
{
    return mAutoScale;
}

void VisDisplayGraph::ToggleDisplayMode()
{
    mpSignalWidget->Display().SetDisplayMode((mpSignalWidget->Display().DisplayMode() + 1) % SignalDisplay::numDisplayModes);
    UpdateGraphActions();
}

void VisDisplayGraph::ToggleNumericValues()
{
    mpSignalWidget->Display().SetNumericValuesVisible(!mpSignalWidget->Display().NumericValuesVisible());
    Visconfigs()[mVisID].Put(CfgID::ShowNumericValues, mpSignalWidget->Display().NumericValuesVisible(), UserDefined);
    UpdateGraphActions();
}

void VisDisplayGraph::ToggleValueUnit()
{
    mpSignalWidget->Display().SetValueUnitVisible(!mpSignalWidget->Display().ValueUnitVisible());
    Visconfigs()[mVisID].Put(CfgID::ShowValueUnit, mpSignalWidget->Display().ValueUnitVisible(), UserDefined);
    UpdateGraphActions();
}

bool VisDisplayGraph::ToggleNumericValues_Enabled() const
{
    return true;
}

bool VisDisplayGraph::ToggleNumericValues_Checked() const
{
    return mpSignalWidget->Display().NumericValuesVisible();
}

void VisDisplayGraph::ToggleBaselines()
{
    mpSignalWidget->Display().SetBaselinesVisible(!mpSignalWidget->Display().BaselinesVisible());
    Visconfigs()[mVisID].Put(CfgID::ShowBaselines, mpSignalWidget->Display().BaselinesVisible(), UserDefined);
    UpdateGraphActions();
}

bool VisDisplayGraph::ToggleBaselines_Enabled() const
{
    return mpSignalWidget->Display().DisplayMode() == SignalDisplay::polyline;
}

bool VisDisplayGraph::ToggleBaselines_Checked() const
{
    return mpSignalWidget->Display().BaselinesVisible();
}

void VisDisplayGraph::ToggleGrid()
{
    mpSignalWidget->Display().SetGridVisible(!mpSignalWidget->Display().GridVisible());
    Visconfigs()[mVisID].Put(CfgID::ShowGrid, mpSignalWidget->Display().GridVisible(), UserDefined);
    UpdateGraphActions();
}

bool VisDisplayGraph::ToggleGrid_Enabled() const
{
    return mpSignalWidget->Display().DisplayMode() == SignalDisplay::polyline;
}

bool VisDisplayGraph::ToggleGrid_Checked() const
{
    return mpSignalWidget->Display().GridVisible();
}

void VisDisplayGraph::ToggleChannelLabels()
{
    mpSignalWidget->Display().SetChannelLabelsVisible(!mpSignalWidget->Display().ChannelLabelsVisible());
    UpdateGraphActions();
}

bool VisDisplayGraph::ToggleValueUnit_Checked() const
{
    return mpSignalWidget->Display().ValueUnitVisible();
}

bool VisDisplayGraph::ToggleValueUnit_Enabled() const
{
    return true;
}

bool VisDisplayGraph::ToggleChannelLabels_Enabled() const
{
    return !mpSignalWidget->Display().ChannelLabels().empty();
}

bool VisDisplayGraph::ToggleChannelLabels_Checked() const
{
    return mpSignalWidget->Display().ChannelLabelsVisible();
}

bool VisDisplayGraph::ToggleChannelList_Enabled() const
{
    return mpChannelList;
}

bool VisDisplayGraph::ToggleChannelList_Checked() const
{
    return mpChannelList->isVisible();
}

void VisDisplayGraph::ToggleColor()
{
    mpSignalWidget->Display().SetColorDisplay(!mpSignalWidget->Display().ColorDisplay());
    UpdateGraphActions();
}

bool VisDisplayGraph::ToggleColor_Enabled() const
{
    return mpSignalWidget->Display().DisplayMode() == SignalDisplay::polyline || mpSignalWidget->Display().DisplayMode() == SignalDisplay::field2d;
}

bool VisDisplayGraph::ToggleColor_Checked() const
{
    return mpSignalWidget->Display().ColorDisplay();
}

bool VisDisplayGraph::Spectrum_Enabled() const
{
#if DISABLE_SPECTRUM
    return false;
#else
    if (!mSignalIsStream)
        return false;

    PhysicalUnit u;
    u.SetGainWithSymbol("1" + mpSignalWidget->Display().SampleUnit());
    return u.Symbol() == "s";
#endif
}

void VisDisplayGraph::FewerSamples()
{
    ++mUserZoom;
    ApplyUserZoom();
    UpdateGraphActions();
}

bool VisDisplayGraph::FewerSamples_Enabled() const
{
    if (mFixedSampleCount)
        return false;
    float nominalSamples = mConfigDisplaySamples * ::pow(2, -mUserZoom - 1);
    return mUserZoom < kMaxUserScaling && nominalSamples >= 2 * mSignalElements;
}

void VisDisplayGraph::MoreSamples()
{
    --mUserZoom;
    ApplyUserZoom();
    UpdateGraphActions();
}

bool VisDisplayGraph::MoreSamples_Enabled() const
{
    if (mFixedSampleCount)
        return false;
    return mUserZoom > -kMaxUserScaling;
}

void VisDisplayGraph::ResetSampleZoom()
{
    mUserZoom = 0;
    ApplyUserZoom();
    UpdateGraphActions();
}

bool VisDisplayGraph::ResetSampleZoom_Enabled() const
{
    if (mFixedSampleCount)
        return false;
    return mUserZoom != 0;
}

void VisDisplayGraph::FewerChannels()
{
    // Round down to the nearest power of 2.
    int n = mpSignalWidget->Display().DisplayGroups(), roundedN = 1;
    while (n >>= 1)
        roundedN <<= 1;

    int newGroups = roundedN;
    if (newGroups == mpSignalWidget->Display().DisplayGroups())
        newGroups /= 2;
    mpSignalWidget->Display().SetDisplayGroups(newGroups);
    UpdateGraphActions();
}

bool VisDisplayGraph::FewerChannels_Enabled() const
{
    return mpSignalWidget->Display().DisplayGroups() > 1;
}

void VisDisplayGraph::MoreChannels()
{
    int maxDisplayGroups = NumVisibleChannelGroups(),
        newDisplayGroups = std::min(maxDisplayGroups, mpSignalWidget->Display().DisplayGroups() * 2);
    mpSignalWidget->Display().SetDisplayGroups(newDisplayGroups);
    UpdateGraphActions();
}

bool VisDisplayGraph::MoreChannels_Enabled() const
{
    return mpSignalWidget->Display().DisplayGroups() < NumVisibleChannelGroups();
}

void VisDisplayGraph::ToggleChannelClickSelect()
{
    mChannelClickSelect = !mChannelClickSelect;
    Visconfigs()[mVisID].Put(CfgID::ClickingTraceSelectsChannel, mChannelClickSelect, UserDefined);
}

bool VisDisplayGraph::ToggleChannelClickSelect_Checked() const
{
    return mChannelClickSelect;
}

double VisDisplayGraph::FilterCaptionToValue(const std::string& inCaption) const
{
    PhysicalUnit unit;
    unit.SetGain(1.0 / NominalUnitsPerSample() / FilterUnitToValue(mpSignalWidget->Display().SampleUnit()));
    unit.SetSymbol("Hz");
    if (unit.IsPhysical(inCaption))
        return unit.PhysicalToRaw(inCaption);
    const char* p = inCaption.c_str();
    while (*p && !std::isdigit(*p))
        ++p;
    return ::atof(p) / unit.Gain();
}

std::string VisDisplayGraph::FilterValueToCaption(double inValue) const
{
    if (inValue <= 0)
        return OFF;
    PhysicalUnit unit;
    unit.SetGain(1.0 / NominalUnitsPerSample() / FilterUnitToValue(mpSignalWidget->Display().SampleUnit()));
    unit.SetSymbol("Hz");
    return unit.RawToPhysical(inValue, unit.PhysicalToRaw("1Hz"));
}

std::string VisDisplayGraph::NormalizeFilterCaption(const std::string& inCaption) const
{
    return FilterValueToCaption(FilterCaptionToValue(inCaption));
}

double VisDisplayGraph::FilterUnitToValue(const std::string &inUnit) const
{
    PhysicalUnit u;
    u.SetGainWithSymbol("1" + inUnit);
    return u.Symbol() == "s" ? u.Gain() : 0;
}

void VisDisplayGraph::LargerFont()
{
    int size = Round(mpSignalWidget->Display().FontSize() * 1.1);
    if (size == mpSignalWidget->Display().FontSize())
        ++size;
    mpSignalWidget->Display().SetFontSize(size);
    Visconfigs()[mVisID].Put(CfgID::FontSize, mpSignalWidget->Display().FontSize(), UserDefined);
    UpdateGraphActions();
}

void VisDisplayGraph::SmallerFont()
{
    int size = Round(mpSignalWidget->Display().FontSize() / 1.1);
    if (size == mpSignalWidget->Display().FontSize())
        --size;
    mpSignalWidget->Display().SetFontSize(size);
    Visconfigs()[mVisID].Put(CfgID::FontSize, mpSignalWidget->Display().FontSize(), UserDefined);
    UpdateGraphActions();
}

void VisDisplayGraph::ToggleChannelList()
{
    if (mpChannelList)
        mpChannelList->setVisible(!mpChannelList->isVisible());
    Visconfigs()[mVisID].Put(CfgID::ShowChannelList, mpChannelList && mpChannelList->isVisible(), UserDefined);
    UpdateGraphActions();
}

bool VisDisplayGraph::SmallerFont_Enabled() const
{
    return mpSignalWidget->Display().FontSize() > 1;
}

int VisDisplayGraph::NominalDisplaySamples() const
{
    return mpSignalWidget->Display().NumSamples() * mDecimationFilter.Decimation();
}

void VisDisplayGraph::SetNominalDisplaySamples(int inSamples)
{
    if (inSamples != NominalDisplaySamples())
    {
        double nominalUnitsPerSample = NominalUnitsPerSample();
        int actualSamples = inSamples, decimation = 1;
        int screenWidth = QGuiApplication::primaryScreen()->geometry().width();
        if (screenWidth < inSamples)
        {
#if AUTO_DECIMATE
            decimation = ::ceil(static_cast<float>(inSamples) / screenWidth / cDisplayOversampling);
#else
            decimation = 1;
#endif
            actualSamples = inSamples / decimation;
        }
        mpSignalWidget->Display().SetNumSamples(actualSamples);
        mDecimationFilter.Decimation(decimation);
        SetNominalUnitsPerSample(nominalUnitsPerSample);
    }
}

double VisDisplayGraph::NominalUnitsPerSample() const
{
    return mpSignalWidget->Display().UnitsPerSample() / mDecimationFilter.Decimation();
}

void VisDisplayGraph::SetNominalUnitsPerSample(double inUps)
{
    mpSignalWidget->Display().SetUnitsPerSample(inUps * mDecimationFilter.Decimation());
}

bool VisDisplayGraph::Filter_Enabled() const
{
    return mActivateFilters && !mFixedScale && NominalUnitsPerSample() * FilterUnitToValue(mpSignalWidget->Display().SampleUnit()) != 0;
}

void VisDisplayGraph::SetHP(const std::string &inCaption, bool inWriteToConfig)
{
    bool ok = true;
    if (inCaption == OFF)
    {
        mDisplayFilter.HPCorner(0);
    }
    else
    {
        double value = FilterCaptionToValue(inCaption.c_str());
        if (value < 0 || value > 0.5) {
            ok = false;
            QString message = QString("Error when setting High Pass corner,\nfrequency is out of range:\n%1").arg(inCaption.c_str());
            QMessageBox::critical(this, "BCI2000Error", message);
        }
        else {
            mDisplayFilter.HPCorner(value);
        }
    }
    if (ok) {
        std::string caption = NormalizeFilterCaption(inCaption);
        mHPMenu.SetCurrentEntry(caption);
        if (inWriteToConfig)
            Visconfigs()[mVisID].Put(CfgID::HPFilter, caption, UserDefined);
    }
    UpdateStatusBar();
    UpdateGraphActions();
}

void VisDisplayGraph::SetHPOff()
{
    SetHP(OFF, true);
}

void VisDisplayGraph::SetHP01()
{
    SetHP("0.1Hz", true);
}

void VisDisplayGraph::SetHP1()
{
    SetHP("1Hz", true);
}

void VisDisplayGraph::SetHP5()
{
    SetHP("5Hz", true);
}

void VisDisplayGraph::SetHPCustom()
{
    QInputDialog dialog(this);
    double corner = mDisplayFilter.HPCorner();
    dialog.setTextValue(FilterValueToCaption(corner).c_str());
    dialog.setWindowTitle("Choose a custom HP filter corner");
    if (dialog.exec()) {
        SetHP(dialog.textValue().toUtf8().constData(), true);
    }
}

void VisDisplayGraph::SetLP(const std::string &inCaption, bool inWriteToConfig)
{
    bool ok = true;
    if (inCaption == OFF)
    {
        mDisplayFilter.LPCorner(0);
    }
    else
    {
        double value = FilterCaptionToValue(inCaption.c_str());
        if (value < 0 || value > 0.5) {
            ok = false;
            QString message = QString("Error when setting Low Pass corner,\nfrequency is out of range:\n%1").arg(inCaption.c_str());
            QMessageBox::critical(this, "BCI2000Error", message);
        }
        else {
            mDisplayFilter.LPCorner(value);
        }
    }
    if (ok) {
        std::string caption = NormalizeFilterCaption(inCaption);
        mLPMenu.SetCurrentEntry(caption);
        if (inWriteToConfig)
            Visconfigs()[mVisID].Put(CfgID::LPFilter, caption, UserDefined);
    }
    UpdateStatusBar();
    UpdateGraphActions();
}

void VisDisplayGraph::SetLPOff()
{
    SetLP(OFF, true);
}

void VisDisplayGraph::SetLP30()
{
    SetLP("30Hz", true);
}

void VisDisplayGraph::SetLP40()
{
    SetLP("40Hz", true);
}

void VisDisplayGraph::SetLP70()
{
    SetLP("70Hz", true);
}

void VisDisplayGraph::SetLPCustom()
{
    QInputDialog dialog(this);
    double corner = mDisplayFilter.LPCorner();
    dialog.setTextValue(FilterValueToCaption(corner).c_str());
    dialog.setWindowTitle("Choose a custom LP filter corner");
    if (dialog.exec()) {
        SetLP(dialog.textValue().toUtf8().constData(), true);
    }
}

void VisDisplayGraph::SetNotch(const std::string &inCaption, bool inWriteToConfig)
{
    if (inCaption == OFF)
    {
        mDisplayFilter.NotchCenter(0);
    }
    else
    {
        mDisplayFilter.NotchCenter(FilterCaptionToValue(inCaption.c_str()));
    }
    std::string caption = NormalizeFilterCaption(inCaption);
    mNotchMenu.SetCurrentEntry(caption);
    if (inWriteToConfig)
        Visconfigs()[mVisID].Put(CfgID::NotchFilter, caption, UserDefined);
    UpdateStatusBar();
    UpdateGraphActions();
}

void VisDisplayGraph::SetNotchOff()
{
    SetNotch(OFF, true);
}

void VisDisplayGraph::SetNotch50()
{
    SetNotch("50Hz", true);
}

void VisDisplayGraph::SetNotch60()
{
    SetNotch("60Hz", true);
}

void VisDisplayGraph::UpdateStatusBar()
{
    std::ostringstream oss;
    oss << "HP: " << mHPMenu.CurrentEntry()
        << "   LP: " << mLPMenu.CurrentEntry()
        << "   Notch: " << mNotchMenu.CurrentEntry();
    mpStatusLabel->setText(QString::fromUtf8(oss.str().c_str()));
}

void VisDisplayGraph::SetXScale(float inScale, bool inWriteToConfig)
{
    mpSignalWidget->Display().SetSampleLogScale(inScale);
    SyncLogScaleMenu(inScale, mpXLogScaleMenu);
    if (inWriteToConfig)
        Visconfigs()[mVisID].Put(CfgID::XAxisLogScale, inScale, UserDefined);
    UpdateGraphActions();
}

void VisDisplayGraph::SetXScaleLinear()
{
    SetXScale(0, true);
}

void VisDisplayGraph::SetXScaleLog10()
{
    SetXScale(10, true);
}

void VisDisplayGraph::SetXScaleLog20()
{
    SetXScale(20, true);
}

void VisDisplayGraph::SetXScaleLog100()
{
    SetXScale(100, true);
}

void VisDisplayGraph::SetXScaleCustom()
{
    QInputDialog dialog(this);
    double scale = mpSignalWidget->Display().SampleLogScale();
    dialog.setDoubleValue(scale);
    dialog.setDoubleRange(0, 1e4);
    dialog.setWindowTitle("Choose a custom log scale factor for the X axis");
    if (dialog.exec()) {
        SetXScale(dialog.doubleValue(), true);
    }
}

void VisDisplayGraph::SetYScale(float inScale, bool inWriteToConfig)
{
    mpSignalWidget->Display().SetValueLogScale(inScale);
    SyncLogScaleMenu(inScale, mpYLogScaleMenu);
    if (inWriteToConfig)
        Visconfigs()[mVisID].Put(CfgID::YAxisLogScale, inScale, UserDefined);
    UpdateGraphActions();
}

void VisDisplayGraph::SetYScaleLinear()
{
    SetYScale(0, true);
}

void VisDisplayGraph::SetYScaleLog10()
{
    SetYScale(10, true);
}

void VisDisplayGraph::SetYScaleLog20()
{
    SetYScale(20, true);
}

void VisDisplayGraph::SetYScaleLog100()
{
    SetYScale(100, true);
}

void VisDisplayGraph::SetYScaleCustom()
{
    QInputDialog dialog(this);
    double scale = mpSignalWidget->Display().SampleLogScale();
    dialog.setDoubleValue(scale);
    dialog.setDoubleRange(0, 1e4);
    dialog.setWindowTitle("Choose a custom log scale factor for the Y axis");
    if (dialog.exec()) {
        SetYScale(dialog.doubleValue(), true);
    }
}

QAction* VisDisplayGraph::SyncLogScaleMenu(float inScale, QMenu* inpMenu)
{
    QAction* pResult = nullptr;
    if (inScale < Eps<float>()) {
        pResult = inpMenu->actions().at(0);
    }
    else {
        QString scaleString = QString::number(inScale);
        for (auto pAction : inpMenu->actions()) {
            bool isCurrent = pAction->text().endsWith(scaleString);
            if (isCurrent) {
                pResult = pAction;
                break;
            }
        }
    }
    for (auto pAction : inpMenu->actions()) {
        pAction->setChecked(pAction == pResult);
    }
    return pResult;
}

void VisDisplayGraph::BuildChannelList()
{
    Assert(mpChannelList == nullptr);
    mpChannelList = new QListWidget;
    mpChannelList->setFrameStyle(QFrame::NoFrame);
    mpChannelList->setFixedWidth(0);
    mpChannelList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mpChannelList->setFocusPolicy(Qt::NoFocus);
    connect(mpChannelList, &QListWidget::itemChanged, this, &VisDisplayGraph::OnChannelListItemChanged);
    connect(mpChannelList, &QListWidget::itemSelectionChanged, this, &VisDisplayGraph::OnChannelListItemSelectionChanged);
}

void VisDisplayGraph::UpdateChannelList(const LabelList& labels)
{
    if (mpChannelList)
    {
        int displayGroups = mpSignalWidget->Display().DisplayGroups();
        mpChannelList->clear();
        mChannelMap.clear();
        for (int i = 0; i < labels.Size(); ++i)
        {
            int index = labels[i].Address();
            mChannelMap[labels[i].Text()] = index;

            QSignalBlocker block(mpChannelList);
            auto pItem = new QListWidgetItem(mpChannelList);
            pItem->setData(Qt::UserRole, QVariant::fromValue(index));
            pItem->setText(QString::fromUtf8(labels[i].Text().c_str()));
            pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable);

            if (!mChannelsVisible.Exists(labels[i].Text()))
                mChannelsVisible[labels[i].Text()] = true;

            Qt::CheckState state = mChannelsVisible[labels[i].Text()] ? Qt::Checked : Qt::Unchecked;
            pItem->setCheckState(state);
            block.unblock();
            mpSignalWidget->Display().SetChannelVisible(index, state == Qt::Checked);
        }
        mpSignalWidget->Display().SetDisplayGroups(displayGroups);
        int width = mpChannelList->sizeHintForColumn(0);
        width += mpChannelList->fontMetrics().horizontalAdvance("  ");
        width += qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        mpChannelList->setFixedWidth(width);

        mChannelsSelected.clear();
        mChannelsSelected.resize(labels.Size());
    }
}

int VisDisplayGraph::NumVisibleChannelGroups() const
{
    std::vector<bool> visible;
    MappedPropertyToContainer(mChannelsVisible, visible);
    int groupSize = mpSignalWidget->Display().ChannelGroupSize();
    int numGroups = (visible.size() + groupSize - 1) / groupSize;
    int count = 0;
    for (int group = 0; group < numGroups; ++group) {
        bool groupVisible = false;
        for (int i = 0; i < groupSize; ++i) {
            int idx = group * groupSize + i;
            if (idx < visible.size() && visible[idx])
                groupVisible = true;
        }
        if (groupVisible)
            ++count;
    }
    return count;
}

bool VisDisplayGraph::ChannelList_Enabled() const
{
    return mpChannelList && mpChannelList->selectedItems().count() > 0;
}

bool VisDisplayGraph::EnlargeSignal_Enabled() const
{
    if (mFixedScale)
        return false;
    if (mAutoScale)
        return false;

    int minScaling = kMaxUserScaling;
    auto selected = ExtendedChannelSelection();
    for (int ch = 0; ch < selected.size(); ++ch) {
        int scaling = GetUserScalingForChannel(ch);
        if (scaling < minScaling)
            minScaling = scaling;
    }
    return minScaling < kMaxUserScaling;
}

bool VisDisplayGraph::ReduceSignal_Enabled() const
{
    if (mFixedScale)
        return false;
    if (mAutoScale)
        return false;

    int maxScaling = -kMaxUserScaling;
    auto selected = ExtendedChannelSelection();
    for (int ch = 0; ch < selected.size(); ++ch) {
        int scaling = GetUserScalingForChannel(ch);
        if (scaling > maxScaling)
            maxScaling = scaling;
    }
    return maxScaling > -kMaxUserScaling;
}

bool VisDisplayGraph::ResetSignalScale_Enabled() const
{
    if (mFixedScale)
        return false;
    if (mAutoScale)
        return false;

    auto selected = ExtendedChannelSelection();
    for (int ch = 0; ch < selected.size(); ++ch) {
        int scaling = GetUserScalingForChannel(ch);
        if (scaling != 0)
            return true;
    }
    return false;
}

void VisDisplayGraph::ChannelList_Enable()
{
    if (mpChannelList)
    {
        auto selected = mpChannelList->selectedItems();
        for (const auto pItem : selected)
            pItem->setCheckState(Qt::Checked);
    }
}

void VisDisplayGraph::ChannelList_Disable()
{
    if (mpChannelList)
    {
        auto selected = mpChannelList->selectedItems();
        for (const auto pItem : selected)
            pItem->setCheckState(Qt::Unchecked);
    }
}

void VisDisplayGraph::EnlargeSignal()
{
    auto selected = ExtendedChannelSelection();
    for (int ch = 0; ch < selected.size(); ++ch)
    {
        if (selected[ch]) {
            auto scaling = GetUserScalingForChannel(ch);
            SetUserScalingForChannel(ch, scaling + 1);
        }
    }
    ApplyUserScaling();
    UpdateGraphActions();
}

void VisDisplayGraph::ReduceSignal()
{
    auto selected = ExtendedChannelSelection();
    for (int ch = 0; ch < selected.size(); ++ch)
    {
        if (selected[ch]) {
            auto scaling = GetUserScalingForChannel(ch);
            SetUserScalingForChannel(ch, scaling - 1);
        }
    }
    ApplyUserScaling();
    UpdateGraphActions();
}

void VisDisplayGraph::ResetSignalScale()
{
    auto selected = ExtendedChannelSelection();
    for (int ch = 0; ch < selected.size(); ++ch)
        if (selected[ch])
            SetUserScalingForChannel(ch, 0);
    ApplyUserScaling();
}

void VisDisplayGraph::OnChannelListItemSelectionChanged()
{
    Assert(mChannelsSelected.size() >= mpChannelList->count());
    for (int row = 0; row < mpChannelList->count(); ++row) {
        bool selected = mpChannelList->item(row)->isSelected();
        mChannelsSelected[row] = selected;
    }
    mpSignalWidget->Display().SetChannelSelected(mChannelsSelected);
}

void VisDisplayGraph::OnChannelListItemChanged(QListWidgetItem* pItem)
{
    auto var = pItem->data(Qt::UserRole);
    if (var.isValid())
    {
        double index = var.toDouble();
        bool checked = (pItem->checkState() == Qt::Checked);
        int groups = mpSignalWidget->Display().DisplayGroups();
        mpSignalWidget->Display().SetChannelVisible(index, checked);
        mpSignalWidget->Display().SetDisplayGroups(groups);
        mChannelsVisible[pItem->text().toUtf8().constData()] = checked;
        Visconfigs()[mVisID].Put(CfgID::ChannelsVisible, mChannelsVisible, UserDefined);
        UpdateGraphActions();
    }
}

void VisDisplayGraph::NewSpectrumDisplay()
{
    auto visID = GetUniqueDependantID("Spectrum");
    CreateSpectrumDisplay(visID);
}

void VisDisplayGraph::SaveDependants()
{
    ValueList<std::string> dependants;
    for (const auto& dependant : Dependants()) {
        dependants.push_back(dependant);
    }
    Visconfigs()[mVisID].Put(CfgID::Dependants, dependants, UserDefined);
}

void VisDisplayGraph::OnDependantCreated(const VisID&)
{
    SaveDependants();
}

void VisDisplayGraph::OnDependantDestroyed(const VisID&)
{
    SaveDependants();
}

void VisDisplayGraph::CreateSpectrumDisplay(const VisID& visID)
{
    VisDisplay::Create(visID, "Spectrum");
    UpdateSpectrumDisplay(visID);
}

void VisDisplayGraph::UpdateSpectrumDisplay(const VisID& visID)
{
    std::string windowTitle = CreateDependantWindowTitle(visID);
    VisDisplay::HandleProperty(visID, CfgID::WindowTitle, windowTitle, MessageDefined);

    auto layer = visID;
    layer.ToLayer();

    VisDisplay::HandleProperty(layer, CfgID::ActivateBuffer, "1");

    const int numBins = 128;
    double maxAbs = std::max(std::fabs(mMinValue), std::fabs(mMaxValue));
    VisDisplay::HandleProperty(layer, CfgID::MinValue, "0", UserDefined);
    VisDisplay::HandleProperty(layer, CfgID::MaxValue, std::to_string(maxAbs * maxAbs / numBins), UserDefined);
    int nominalDisplaySamples = NominalDisplaySamples();
    int decimation = 1.0 / 20 / mElementGain; // decimate to 20 spectra per second
    if (decimation < 1) {
        decimation = 1;
    }
    else if (decimation > nominalDisplaySamples) {
        decimation = nominalDisplaySamples;
    }
    const auto& display = mpSignalWidget->Display();
    std::string sampleUnit = std::to_string(mElementGain) + display.SampleUnit();
    VisDisplay::HandleProperty(layer, CfgID::SampleUnit, sampleUnit, MessageDefined);
    VisDisplay::HandleProperty(layer, CfgID::NumSamples, std::to_string(nominalDisplaySamples / decimation), MessageDefined);
    VisDisplay::HandleProperty(layer, CfgID::AutoScale, "1", Default);
    VisDisplay::HandleProperty(layer, CfgID::DftDecimate, std::to_string(decimation), MessageDefined);
    VisDisplay::HandleProperty(layer, CfgID::DftNumBins, std::to_string(numBins), MessageDefined);
    VisDisplay::HandleProperty(layer, CfgID::DftType, std::to_string(CfgID::DftLinear), Default);
    VisDisplay::HandleProperty(layer, CfgID::ShowChannelList, "1", Default);
    std::ostringstream oss;
    oss << mpSignalWidget->Display().ChannelLabels();
    VisDisplay::HandleProperty(layer, CfgID::ChannelLabels, oss.str(), MessageDefined);

    VisDisplay::HandleProperty(layer, CfgID::ActivateBuffer, "0");
}

void VisDisplayGraph::ChannelList_DisableAll()
{
    if (mpChannelList)
    {
        for (int i = 0; i < mpChannelList->count(); ++i)
            mpChannelList->item(i)->setCheckState(Qt::Unchecked);
    }
}

void VisDisplayGraph::ChannelList_EnableAll()
{
    if (mpChannelList)
    {
        for (int i = 0; i < mpChannelList->count(); ++i)
            mpChannelList->item(i)->setCheckState(Qt::Checked);
    }
}

void VisDisplayGraph::ToggleColorSelector()
{
    if (!mpColorSelector) {
        mpColorSelector = new ExtendedColorSelector;
        QString windowTitle = parentWidget() ? parentWidget()->windowTitle() : "";
        if (!windowTitle.isEmpty())
            windowTitle = "Choose Channel Colors for \"" + windowTitle + "\"...";
        else
            windowTitle = "Choose Channel Colors";
        mpColorSelector->setWindowTitle(windowTitle);
        connect(mpColorSelector, &ExtendedColorSelector::colorApplied, this, &VisDisplayGraph::ApplyColorSelection);
        connect(mpColorSelector, &ExtendedColorSelector::colorPatternApplied, this, &VisDisplayGraph::ApplyColorPattern);
        connect(mpColorSelector, &ExtendedColorSelector::resetClicked, this, &VisDisplayGraph::ApplyColorReset);
        connect(mpColorSelector, &ExtendedColorSelector::closed, this, &VisDisplayGraph::ColorSelectorClosed);
    }
    mpColorSelector->setVisible(!mpColorSelector->isVisible());
    if (mpColorSelector->isVisible()) {
        mpColorSelector->setApplyColorButtonEnabled(AnyChannelsSelected());
        mpColorSelector->activateWindow();
        // make a list of unique channel colors to populate the custom colors array
        ColorList channelColors = mpSignalWidget->Display().ChannelColors();
        std::vector<QColor> uniqueColors;
        for (auto color : channelColors) {
            QColor c = color.ToQRgb();
            if (std::find(uniqueColors.begin(), uniqueColors.end(), c) == uniqueColors.end())
                uniqueColors.push_back(c);
        }
        mpColorSelector->setCustomColors(uniqueColors);
        mOriginalColors = channelColors;
    }
}

bool VisDisplayGraph::ToggleColorSelector_Enabled() const
{
    return mpSignalWidget->Display().ColorDisplay() 
        && mpSignalWidget->Display().DisplayMode() == SignalDisplay::polyline
        && mpSignalWidget->Display().ChannelGroupSize() == 1;
}

void VisDisplayGraph::ColorSelectorClosed()
{
    mpActToggleColorSelector->setChecked(false);
}

void VisDisplayGraph::ApplyColorSelection(const QColor& color)
{
    ColorList channelColors = mpSignalWidget->Display().ChannelColors();
    int oldSize = channelColors.size();
    for (int i = oldSize; i < mChannelsSelected.size(); ++i)
        channelColors.push_back(channelColors[i - oldSize]);
    for (int i = 0; i < mChannelsSelected.size(); ++i)
        if (mChannelsSelected[i])
            channelColors[i] = RGBColor::FromQRgb(color.rgba());
    mpSignalWidget->Display().SetChannelColors(channelColors);
    Visconfigs()[mVisID].Put(CfgID::ChannelColors, channelColors, UserDefined);
}

void VisDisplayGraph::ApplyColorPattern(const QString& countStr)
{
    int count = countStr.toInt();
    if (count > 1) {
        ColorList channelColors = mpSignalWidget->Display().ChannelColors();
        int oldSize = channelColors.size();
        for (int i = oldSize; i < count; ++i)
            channelColors.push_back(channelColors[i - oldSize]);
        channelColors.resize(count);
        mpSignalWidget->Display().SetChannelColors(channelColors);
        Visconfigs()[mVisID].Put(CfgID::ChannelColors, channelColors, UserDefined);
    }
}

void VisDisplayGraph::ApplyColorReset()
{
    mpSignalWidget->Display().SetChannelColors(mOriginalColors);
}

void VisDisplayGraph::ColorThemeAuto()
{
    mColorTheme = auto_;
    Visconfigs()[mVisID].Put(CfgID::ColorTheme, mColorTheme, UserDefined);
    ApplyColorTheme();
    UpdateGraphActions();
}

void VisDisplayGraph::ColorThemeLight()
{
    mColorTheme = light_;
    Visconfigs()[mVisID].Put(CfgID::ColorTheme, mColorTheme, UserDefined);
    ApplyColorTheme();
    UpdateGraphActions();
}

void VisDisplayGraph::ColorThemeDark()
{
    mColorTheme = dark_;
    Visconfigs()[mVisID].Put(CfgID::ColorTheme, mColorTheme, UserDefined);
    ApplyColorTheme();
    UpdateGraphActions();
}

void VisDisplayGraph::SetChannelSelected(int ch, bool selected)
{
    Assert(ch < mChannelsSelected.size());
    mChannelsSelected[ch] = selected;
    UpdateChannelSelection();
}

void VisDisplayGraph::ExtendChannelSelection(int ch)
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

bool VisDisplayGraph::ToggleChannelSelected(int ch)
{
    Assert(ch < mChannelsSelected.size());
    mChannelsSelected[ch] = !mChannelsSelected[ch];
    UpdateChannelSelection();
    return mChannelsSelected[ch];
}

void VisDisplayGraph::UpdateChannelSelection()
{
    if (mpChannelList)
    {
        bool oldState = mpChannelList->blockSignals(true);
        for (int row = 0; row < mpChannelList->count(); ++row) {
            mpChannelList->item(row)->setSelected(mChannelsSelected[row]);
        }
        mpChannelList->blockSignals(oldState);
    }
    mpSignalWidget->Display().SetChannelSelected(mChannelsSelected);
    if (mpColorSelector)
        mpColorSelector->setApplyColorButtonEnabled(AnyChannelsSelected());
}

bool VisDisplayGraph::IsChannelVisible(int ch) const
{
    if (mpChannelList && mpChannelList->count() > ch) {
        return mpChannelList->item(ch)->checkState();
    }
    return true;
}

void VisDisplayGraph::SelectAllChannels()
{
    for (int i = 0; i < mChannelsSelected.size(); ++i) {
        mChannelsSelected[i] = true;
    }
    UpdateChannelSelection();
}

void VisDisplayGraph::ClearChannelSelection()
{
    for (int i = 0; i < mChannelsSelected.size(); ++i) {
        mChannelsSelected[i] = false;
    }
    UpdateChannelSelection();
}

bool VisDisplayGraph::AnyChannelsSelected() const
{
    for (bool b : mChannelsSelected)
        if (b)
            return true;
    return false;
}

std::vector<bool> VisDisplayGraph::ExtendedChannelSelection() const
{
    auto selected = mChannelsSelected;
    bool found = false;
    int groupSize = mpSignalWidget->Display().ChannelGroupSize();
    for (int ch = 0; ch < selected.size(); ++ch) {
        // extend selection to entire group
        if (selected[ch]) {
            found = true;
            int begin = (ch / groupSize) * groupSize,
                end = begin + groupSize;
            for (int ch2 = begin; ch2 < end; ++ch2) {
                selected[ch2] = true;
            }
            ch = end;
        }
    }
    if (!found) { // if nothing is selected, apply to all
        for (int ch = 0; ch < selected.size(); ++ch)
            selected[ch] = true;
    }
    return selected;
}

std::pair<int, int> VisDisplayGraph::YRangeToChannels(float y0, float y1) const
{
    if (y0 > y1)
        return YRangeToChannels(y1, y0);
    int count = mpSignalWidget->Display().NumChannels();
    int min = mpSignalWidget->Display().ChannelTop(0);
    int max = mpSignalWidget->Display().ChannelBottom(count - 1);
    if (y0 < min)
        return YRangeToChannels(min, y1);
    if (y1 > max)
        return YRangeToChannels(y0, max);
    int minChannel = mpSignalWidget->Display().PosToChannel(y0);
    int maxChannel = mpSignalWidget->Display().PosToChannel(y1);
    return std::make_pair(minChannel, maxChannel);
}

void VisDisplayGraph::ApplyUserScaling()
{
    int channels = mChannelUserScaling.size();
    if (mMinValues.size() < channels)
        mMinValues.resize(channels, mMinValue);
    if (mMaxValues.size() < channels)
        mMaxValues.resize(channels, mMaxValue);
    std::vector<double> finalMinValues(channels), finalMaxValues(channels);

    double factor1 = ::pow(2.0, -mUserScaling);
    for (int ch = 0; ch < channels; ++ch)
    {
        double factor2 = ::pow(2.0, -mChannelUserScaling[ch]);
        finalMinValues[ch] = mMinValues[ch] * factor1 * factor2;
        finalMaxValues[ch] = mMaxValues[ch] * factor1 * factor2;
    }
    mpSignalWidget->Display().SetMinValues(finalMinValues).SetMaxValues(finalMaxValues);

    auto& config = Visconfigs()[mVisID];
    config.Put(CfgID::UserScaling, mUserScaling, UserDefined);

    MappedProperty<double> userScalingByName;
    config.Get(CfgID::ChannelUserScaling, userScalingByName);
    ContainerToMappedProperty(mChannelUserScaling, userScalingByName);
    config.Put(CfgID::ChannelUserScaling, userScalingByName, UserDefined);
}

void VisDisplayGraph::SetUserScalingForChannel(int channel, int value)
{
    if (mChannelUserScaling.size() <= channel)
        mChannelUserScaling.resize(channel + 1, 0);
    if (value > kMaxUserScaling)
        value = kMaxUserScaling;
    if (value < -kMaxUserScaling)
        value = -kMaxUserScaling;
    mChannelUserScaling[channel] = value;
}

int VisDisplayGraph::GetUserScalingForChannel(int channel) const
{
    if (mChannelUserScaling.size() <= channel)
        return 0;
    return mChannelUserScaling[channel];
}

void VisDisplayGraph::ApplyAutoScale(const PhysicalUnit& unit)
{
    if (mAutoScale)
    {
        SignalDisplay& display = mpSignalWidget->Display();
        int autoScaleSamples = display.NumSamples();
        if (unit.Symbol() == "s")
            autoScaleSamples = unit.PhysicalToRaw(mAutoScaleTime);
        autoScaleSamples = std::max(1, autoScaleSamples);
        autoScaleSamples = std::min(display.NumSamples(), autoScaleSamples);
        int numChannels = display.NumChannels();
        std::vector<double> displayMin(numChannels), displayMax(numChannels);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            double sigMax = -Infinity<double>(), sigMin = Infinity<double>();
            for (int sample = 0; sample < autoScaleSamples; ++sample)
            {
                auto value = display.Data(ch, sample);
                if (!std::isnan(value))
                {
                    sigMax = std::max(sigMax, value);
                    sigMin = std::min(sigMin, value);
                }
            }
            if (sigMin > 0)
            {
                sigMin = 0;
            }
            else if (sigMax < 0)
            {
                sigMax = 0;
            }
            else
            {
                float absMax = std::max(abs(sigMax), abs(sigMin));
                sigMax = absMax;
                sigMin = -absMax;
            }
            if (sigMax != sigMin)
            {
                displayMax[ch] = sigMax * kAutoScaleFactor;
                displayMin[ch] = sigMin * kAutoScaleFactor;
            }
        }
        display.SetMaxValues(displayMax);
        display.SetMinValues(displayMin);
    }
}

void VisDisplayGraph::SaveValueUnitsVisible() const
{
    auto& config = Visconfigs()[mVisID];
    std::vector<bool> valueUnitsVisible = mpSignalWidget->Display().ValueUnitsVisible();
    MappedProperty<bool> valueUnitsVisibleByName;
    config.Get(CfgID::ValueUnitsVisible, valueUnitsVisibleByName);
    ContainerToMappedProperty(valueUnitsVisible, valueUnitsVisibleByName);
    config.Put(CfgID::ValueUnitsVisible, valueUnitsVisibleByName, UserDefined);
}

void VisDisplayGraph::ApplyUserZoom()
{
    float newNominalSamples = mConfigDisplaySamples * ::pow(2, -mUserZoom);
    if (newNominalSamples >= 2 * mSignalElements)
        SetNominalDisplaySamples(newNominalSamples);
    Visconfigs()[mVisID].Put(CfgID::UserSampleZoom, mUserZoom, UserDefined);
}

void VisDisplayGraph::ApplyColorTheme()
{
    bool lightMode = (mColorTheme == light_);
    if (mColorTheme == auto_)
        lightMode = !IsPaletteDark();
    mpSignalWidget->Display().SetLightMode(lightMode);
}

void VisDisplayGraph::hideEvent(QHideEvent *iopEvent)
{
    if (mpColorSelector)
        mpColorSelector->close();
    VisDisplayBase::hideEvent(iopEvent);
    iopEvent->accept();
}

void VisDisplayGraph::moveEvent(QMoveEvent *iopEvent)
{
    VisDisplayBase::moveEvent(iopEvent);
    iopEvent->accept();
}

void VisDisplayGraph::resizeEvent(QResizeEvent *iopEvent)
{
    VisDisplayBase::resizeEvent(iopEvent);
    iopEvent->accept();
}

void VisDisplayGraph::keyReleaseEvent(QKeyEvent *iopEvent)
{
    static int acc = 0;
    int wipe_acc = 1;
    int key = iopEvent->key(), modkey = iopEvent->modifiers();

    if (key == Qt::Key_Space && (modkey & Qt::ShiftModifier))
        key = Qt::Key_PageUp;
    if (acc == 0 && key == Qt::Key_G && (modkey & Qt::ShiftModifier))
        key = Qt::Key_End;
    if (acc != 0 && key == Qt::Key_Return)
        key = Qt::Key_G;

    switch (key)
    {
    case Qt::Key_Up:
        mpSignalWidget->Display().SetTopGroup(mpSignalWidget->Display().TopGroup() - 1);
        break;
    case Qt::Key_Down:
        mpSignalWidget->Display().SetTopGroup(mpSignalWidget->Display().TopGroup() + 1);
        break;
    case Qt::Key_PageUp:
    case Qt::Key_B:
        mpSignalWidget->Display().SetTopGroup(mpSignalWidget->Display().TopGroup() - mpSignalWidget->Display().DisplayGroups());
        break;
    case Qt::Key_PageDown:
    case Qt::Key_Space:
        mpSignalWidget->Display().SetTopGroup(mpSignalWidget->Display().TopGroup() + mpSignalWidget->Display().DisplayGroups());
        break;
    case Qt::Key_Right:
        if (FewerSamples_Enabled())
            FewerSamples();
        break;
    case Qt::Key_Left:
        if (MoreSamples_Enabled())
            MoreSamples();
        break;
    case Qt::Key_Less:
    case Qt::Key_Comma:
        if (FewerChannels_Enabled())
            FewerChannels();
        break;
    case Qt::Key_Greater:
    case Qt::Key_Period:
        if (MoreChannels_Enabled())
            MoreChannels();
        break;
    case Qt::Key_Minus:
        if (ReduceSignal_Enabled())
            ReduceSignal();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        if (EnlargeSignal_Enabled())
            EnlargeSignal();
        break;
    case Qt::Key_Home:
        mpSignalWidget->Display().SetTopGroup(0);
        break;
    case Qt::Key_End:
        mpSignalWidget->Display().SetTopGroup(mNumChannels / mpSignalWidget->Display().ChannelGroupSize());
        break;
    case Qt::Key_G:
        mpSignalWidget->Display().SetTopGroup(acc - 1);
        break;
    case Qt::Key_H:
        if (mActivateFilters) {
            if (modkey & Qt::ShiftModifier)
            {
                std::string entry = mHPMenu.NextEntry();
                SetHP(entry, true);
            }
            else
            {
                if (mDisplayFilter.HPCorner() == 0.0)
                    SetHP(mHPMenu.LastNonnullEntry(), true);
                else
                    SetHP(OFF, true);
            }
        }
        break;
    case Qt::Key_L:
        if (mActivateFilters) {
            if (modkey & Qt::ShiftModifier)
            {
                std::string entry = mLPMenu.NextEntry();
                SetLP(entry, true);
            }
            else
            {
                if (mDisplayFilter.LPCorner() == 0.0)
                    SetLP(mLPMenu.LastNonnullEntry(), true);
                else
                    SetLP(OFF, true);
            }
        }
        break;
    case Qt::Key_N:
        if (mActivateFilters) {
            if (modkey & Qt::ShiftModifier)
            {
                std::string entry = mNotchMenu.NextEntry();
                SetNotch(entry, true);
            }
            else
            {
                if (mDisplayFilter.NotchCenter() == 0.0)
                    SetNotch(mNotchMenu.LastNonnullEntry(), true);
                else
                    SetNotch(OFF, true);
            }
        }
        break;
    case Qt::Key_A:
        SelectAllChannels();
        break;
    case Qt::Key_Escape:
        ClearChannelSelection();
        mpSignalWidget->clearFocus();
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
        acc = acc * 10 + (key - Qt::Key_0);
        wipe_acc = 0;
        break;
    default:
        VisDisplayBase::keyReleaseEvent(iopEvent);
    }
    if (wipe_acc)
        acc = 0;
}

void VisDisplayGraph::mousePressEvent(QMouseEvent *iopEvent)
{
    if (mChannelClickSelect && iopEvent->buttons() == Qt::LeftButton) {
        QPointF where = iopEvent->localPos();
        where -= mpSignalWidget->pos();
        int sample = mpSignalWidget->Display().PosToSample(where.x());
        int ch = mpSignalWidget->Display().PosToChannel(where.y());
        if (ch < 0 || ch >= mChannelsSelected.size() || sample < 0 || sample >= mpSignalWidget->Display().NumSamples()) {
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
    VisDisplayBase::mousePressEvent(iopEvent);
}

void VisDisplayGraph::mouseMoveEvent(QMouseEvent* iopEvent)
{
    if (mChannelClickSelect && iopEvent->buttons() == Qt::LeftButton) {
        QPointF where = iopEvent->localPos();
        where -= mpSignalWidget->pos();
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
    VisDisplayBase::mouseMoveEvent(iopEvent);
}

void VisDisplayGraph::wheelEvent(QWheelEvent* iopEvent)
{
    float yDelta = iopEvent->angleDelta().y();
    if (yDelta > 0) {
        mpSignalWidget->Display().SetTopGroup(mpSignalWidget->Display().TopGroup() - 1);
        iopEvent->accept();
    }
    else if (yDelta < 0) {
        mpSignalWidget->Display().SetTopGroup(mpSignalWidget->Display().TopGroup() + 1);
        iopEvent->accept();
    }
    else {
        iopEvent->ignore();
    }
    VisDisplayBase::wheelEvent(iopEvent);
}

void VisDisplayGraph::changeEvent(QEvent *iopEvent)
{
    if (iopEvent->type() == QEvent::PaletteChange) {
        PopulateToolBar();
        ApplyColorTheme();
    }
    VisDisplayBase::changeEvent(iopEvent);
}

VisDisplayGraph::FilterMenu::FilterMenu()
: mFixedEntries(&EntryCmp)
{
    mLastNonnullEntry = OFF;
    mCurrentEntry = OFF;
}

void VisDisplayGraph::FilterMenu::SetMenu(QMenu* pMenu)
{
    mFixedEntries.clear();
    mpMenu = pMenu;
    for (int i = 0; i < pMenu->actions().size(); ++i) {
        auto pAction = pMenu->actions().at(i);
        if (pAction->isSeparator())
            break;
        mFixedEntries.insert(pAction->text().toUtf8().constData());
    }
}

std::string VisDisplayGraph::FilterMenu::NextEntry() const
{
    auto allEntries = this->mFixedEntries;
    if (!mCustomEntry.empty())
        allEntries.insert(this->mCustomEntry);
    auto i = allEntries.find(this->CurrentEntry());
    if (i == allEntries.end())
        return OFF;
    if (++i == allEntries.end())
        i = allEntries.begin();
    return *i;
}

// Compare corner frequency entries in filter menus
bool VisDisplayGraph::FilterMenu::EntryCmp(const std::string& a, const std::string& b)
{
    PhysicalUnit unit;
    unit.SetGainWithSymbol("1Hz");
    if (!unit.IsPhysical(a))
        return unit.IsPhysical(b);
    if (!unit.IsPhysical(b))
        return false;
    return unit.PhysicalToRaw(a) < unit.PhysicalToRaw(b);
}

void VisDisplayGraph::FilterMenu::SetCurrentEntry(const std::string& inCaption)
{
    for (int i = 0; i < mpMenu->actions().size(); ++i)
    {
        QAction* pAction = mpMenu->actions().at(i);
        std::string caption = pAction->text().toUtf8().constData();
        pAction->setChecked(inCaption == caption);
    }
    if (this->mFixedEntries.find(inCaption) == this->mFixedEntries.end())
        this->mCustomEntry = inCaption;
    if (inCaption != OFF)
        this->mLastNonnullEntry = inCaption;
    this->mCurrentEntry = inCaption;
}

template<class T>
void VisDisplayGraph::MappedPropertyToContainer(const MappedProperty<T>& map, std::vector<T>& container) const
{
    container.resize(mChannelMap.size());
    for (const auto& entry : map)
    {   // mChannelMap provides Channel Name->Channel Index
        auto i = mChannelMap.find(entry.first);
        if (i != mChannelMap.end())
        {
            if (i->second >= container.size())
                container.resize(i->second + 1);
            container[i->second] = entry.second;
        }
    }
}

template<class T>
void VisDisplayGraph::ContainerToMappedProperty(const std::vector<T>& container, MappedProperty<T>& map) const
{
    map.Clear();
    // mChannelMap provides Channel Name->Channel Index
    for (const auto& entry : mChannelMap)
        if (entry.second < container.size())
            map[entry.first] = container[entry.second];
}

