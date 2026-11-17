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
#include "VisDisplaySpectrum.h"

#include "Resources.h"

#include "ToolButtonAction.h"

#include <limits>
#include <cmath>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QInputDialog>
#include <QLayout>
#include <QMenu>
#include <QToolBar>
#include <QListWidget>
#include <QHideEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QKeyEvent>

static const int kMaxUserScaling = 32; // The maximum number of scaling steps a user can take from the default.
static const int kAutoDecimationMaxLinesPerPixel = 5;
static const int kDisplaySamplesDefault = 50;
static const double kDftQFactorDefault = 50;
static const int kDftNumBinsDefault = 100;
static const int kSignalQueueSizeLimit = 5; // Signal queue will be flushed when size limit is reached.

VisDisplaySpectrum::VisDisplaySpectrum(const std::string &inSourceID)
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
    BuildGraphContextMenu();
    BuildChannelList();
    BuildChannelListContextMenu();
    PopulateToolBar();
    pHLayout->addWidget(mpChannelList);
    setLayout(pVLayout);

    mProcessingThread = std::thread([this]{ProcessingThreadFunc();});
}

VisDisplaySpectrum::~VisDisplaySpectrum()
{
    mTerminateProcessingThread = true;
    mSignalQueueCondition.notify_all();
    if (mProcessingThread.joinable()) {
        mProcessingThread.join();
    }
    delete mpGraphContextMenu;
    delete mpChannelListContextMenu;
}

void VisDisplaySpectrum::OnSetConfig(ConfigSettings &inConfig)
{
    VisDisplayBase::OnSetConfig(inConfig);

    SignalDisplay& display = mpSignalWidget->Display();

    double minValue, maxValue;
    if (inConfig.Get(CfgID::MinValue, minValue))
        mMinValue = minValue;
    if (inConfig.Get(CfgID::MaxValue, maxValue))
        mMaxValue = maxValue;

    double userScaling = 0;
    if (inConfig.Get(CfgID::UserScaling, userScaling))
        mUserScaling = userScaling;
    ApplyUserScaling();

    inConfig.Get(CfgID::FixedSampleCount, mFixedSampleCount);

    int numSamples = DisplaySamples();
    if (inConfig.Get(CfgID::NumSamples, numSamples))
    {
        mConfigDisplaySamples = numSamples;
        SetDisplaySamples(numSamples);
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

    display.SetChannelGroupSize(1);
    display.SetDisplayMode(SignalDisplay::field2d);

    bool autoScale = true;
    inConfig.Get(CfgID::AutoScale, autoScale);
    mAutoScaleFilter.enabled = autoScale;

    bool showNumericValues = display.NumericValuesVisible();
    if (inConfig.Get(CfgID::ShowNumericValues, showNumericValues))
        display.SetNumericValuesVisible(showNumericValues);

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

    std::string channelStyles = display.ChannelStyles();
    if (inConfig.Get(CfgID::ChannelStyles, channelStyles))
        display.SetChannelStyles(channelStyles);

    std::string valueColors;
    if (inConfig.Get(CfgID::ValueColors, valueColors))
        display.SetValueColors(valueColors);

    std::string unit;
    float unitsPerSample = 1;
    if (inConfig.Get(CfgID::SampleUnit, unit))
    {
        PhysicalUnit u;
        u.SetGainWithSymbol(unit);
        unitsPerSample = u.Gain();
        std::string sampleUnit = u.Symbol();
        display.SetSampleUnit(u.Symbol());
        double elementGain = unitsPerSample * UnitToValue(sampleUnit);
        mInputSamplingRate = 1.0 / elementGain;
    }
    float sampleOffset;
    if (inConfig.Get(CfgID::SampleOffset, sampleOffset))
        display.SetSampleOffset(-sampleOffset);

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

    float yAxisLogScale = display.ValueLogScale();
    if (inConfig.Get(CfgID::YAxisLogScale, yAxisLogScale))
        SetValueScale(yAxisLogScale);

    bool showToolBar = true;
    inConfig.Get(CfgID::ShowToolBar, showToolBar);
    ShowToolBar(showToolBar);

    bool showChannelList = true;
    inConfig.Get(CfgID::ShowChannelList, showChannelList);
    if (mpChannelList)
        mpChannelList->setVisible(showChannelList);

    int dftType = CfgID::DftConstantQ;
    inConfig.Get(CfgID::DftType, dftType);
    switch (dftType) {
        case CfgID::DftLinear:
            mDftType = linear;
            break;
        case CfgID::DftConstantQ:
            mDftType = constantQ;
            break;
        default:
            SuggestDebugging << "Unexpected DFT type";
    }

    int dftNumBins = kDftNumBinsDefault;
    inConfig.Get(CfgID::DftNumBins, dftNumBins);
    mDftNumBins = dftNumBins;

    int dftDecimation = 1;
    inConfig.Get(CfgID::DftDecimate, dftDecimation);
    mDftDecimation = dftDecimation;

    InitializeDftConfig();
    SetUnitsPerSample(unitsPerSample * mDftSpectraDecimation * mDecimationFilter.Decimation());

    // Sanity check.
    if (display.MinValue() == display.MaxValue())
        display.SetMaxValue(display.MinValue() + 1);

    InitializeAutoScale();

    UpdateGraphActions();
    UpdateChannelListActions();
}

void VisDisplaySpectrum::OnSignal(const GenericSignal &s)
{
    if (s.Channels() < 1 || s.Elements() < 1)
        return;

    SignalDisplay& display = mpSignalWidget->Display();
    int curChannels = mNumChannels, curSamples = display.NumSamples();
    mNumChannels = s.Channels();
    if (curChannels != mNumChannels || curSamples != display.NumSamples())
        OnSetConfig(Visconfigs()[mVisID]);

    std::unique_lock<std::mutex> lock(mDftMutex);
    if (mDftObserver.BankCount() != s.Channels()) {
        InitializeDft();
    }
    ApplyAutoScale();
    lock.unlock();

    std::unique_lock<std::mutex> signalQueueLock(mSignalQueueMutex);
    mSignalQueue.push(s);
    signalQueueLock.unlock();
    mSignalQueueCondition.notify_one();
}

void VisDisplaySpectrum::ProcessingThreadFunc()
{
    while (!mTerminateProcessingThread) {
        std::unique_lock<std::mutex> lock(mSignalQueueMutex);
        mSignalQueueCondition.wait(lock, [this] {return !mSignalQueue.empty() || mTerminateProcessingThread; });
        if (mSignalQueue.size() > kSignalQueueSizeLimit) {
            while (!mSignalQueue.empty())
                mSignalQueue.pop();
        }
        if (!mTerminateProcessingThread && !mSignalQueue.empty()) {
            auto signal = mSignalQueue.front();
            mSignalQueue.pop();
            lock.unlock();

            std::unique_lock<std::mutex> dftLock(mDftMutex);
            mLpFilteredSignal.SetProperties(signal.Properties());
            mDecimationLp.Process(signal, mLpFilteredSignal);
            mDecimationFilter.Process(mLpFilteredSignal, mDecimatedSignal);
            mDftObserver.Process(mDecimatedSignal, &VisDisplaySpectrum::OnDftData, this);
        }
    }
}

void VisDisplaySpectrum::OnDftData(const GenericSignal& spectrum, void* pData)
{
    VisDisplaySpectrum* this_ = reinterpret_cast<VisDisplaySpectrum*>(pData);
    this_->ObserveAutoScale(spectrum);
    this_->mSpectrumSize = spectrum.Channels();
    this_->mpSignalWidget->Display().WrapForward(spectrum);
}

void VisDisplaySpectrum::LoadActionIcons()
{
    mpFrequencyScaleButton->setIcon(LoadIcon(Resources::logarithmic_icon));

    mpActToggleAutoScale->setIcon(LoadIcon(Resources::autoscale_icon));
    mpActEnlargeSignal->setIcon(LoadIcon(Resources::zoom_in_v_icon));
    mpActReduceSignal->setIcon(LoadIcon(Resources::zoom_out_v_icon));
    mpActResetSignalScale->setIcon(LoadIcon(Resources::zoom_reset_v_icon));

    mpActFewerSamples->setIcon(LoadIcon(Resources::zoom_in_h_icon));
    mpActMoreSamples->setIcon(LoadIcon(Resources::zoom_out_h_icon));
    mpActResetSampleZoom->setIcon(LoadIcon(Resources::zoom_reset_h_icon));

    mpActMoreFrequencies->setIcon(LoadIcon(Resources::plus_icon));
    mpActFewerFrequencies->setIcon(LoadIcon(Resources::minus_icon));

    if (mpColorThemeButton)
        mpColorThemeButton->setIcon(LoadIcon(Resources::light_dark_icon));
    if (mpValueLogScaleButton)
        mpValueLogScaleButton->setIcon(LoadIcon(Resources::logarithmic_icon));
}

void VisDisplaySpectrum::BuildGraphContextMenu()
{
    mpGraphContextMenu = new QMenu;

    mpFrequencyScaleMenu = new QMenu("Frequency Scale", mpGraphContextMenu);
    QActionGroup* pCDFTGroup = new QActionGroup(mpFrequencyScaleMenu);
    QAction* pAct = mpFrequencyScaleMenu->addAction("Linear Frequency Scale", this, &VisDisplaySpectrum::SetDftUniform);
    pAct->setCheckable(true);
    pAct->setChecked(mDftType == linear);
    pCDFTGroup->addAction(pAct);
    mpActFrequencyScaleLinear = pAct;
    pAct = mpFrequencyScaleMenu->addAction("Logarithmic Frequency Scale", this, &VisDisplaySpectrum::SetDftConstantQ);
    pAct->setCheckable(true);
    pAct->setChecked(mDftType == constantQ);
    pCDFTGroup->addAction(pAct);
    mpActFrequencyScaleConstantQ = pAct;
    mpGraphContextMenu->addMenu(mpFrequencyScaleMenu);

    mpGraphContextMenu->addSeparator();

    mpActEnlargeSignal = mpGraphContextMenu->addAction("Enlarge Signal", this, &VisDisplaySpectrum::EnlargeSignal);
    mpActReduceSignal = mpGraphContextMenu->addAction("Reduce Signal", this, &VisDisplaySpectrum::ReduceSignal);
    mpActResetSignalScale = mpGraphContextMenu->addAction("Reset Signal Scale", this, &VisDisplaySpectrum::ResetSignalScale);
    mpActToggleAutoScale = mpGraphContextMenu->addAction("Auto Scale", this, &VisDisplaySpectrum::ToggleAutoScale);
    mpActToggleAutoScale->setCheckable(true);

    mpValueLogScaleMenu = new QMenu("Value Scale", mpGraphContextMenu);
    QActionGroup* pValueLogScaleGroup = new QActionGroup(mpValueLogScaleMenu);
    pAct = mpValueLogScaleMenu->addAction("linear", this, SLOT(SetValueScaleLinear()));
    pAct->setCheckable(true);
    pValueLogScaleGroup->addAction(pAct);
    pAct->setChecked(true);
    pAct = mpValueLogScaleMenu->addAction("log10", this, SLOT(SetValueScaleLog10()));
    pAct->setCheckable(true);
    pValueLogScaleGroup->addAction(pAct);
    pAct = mpValueLogScaleMenu->addAction("log20", this, SLOT(SetValueScaleLog20()));
    pAct->setCheckable(true);
    pValueLogScaleGroup->addAction(pAct);
    pAct = mpValueLogScaleMenu->addAction("log100", this, SLOT(SetValueScaleLog100()));
    pAct->setCheckable(true);
    pValueLogScaleGroup->addAction(pAct);
    mpValueLogScaleMenu->addSeparator();
    pAct = mpValueLogScaleMenu->addAction("Other...", this, SLOT(SetValueScaleCustom()));
    pAct->setCheckable(false);
    mpGraphContextMenu->addMenu(mpValueLogScaleMenu);
    mpGraphContextMenu->addSeparator();

    mpActFewerSamples = mpGraphContextMenu->addAction("Fewer Samples", this, &VisDisplaySpectrum::FewerSamples);
    mpActMoreSamples = mpGraphContextMenu->addAction("More Samples", this, &VisDisplaySpectrum::MoreSamples);
    mpActResetSampleZoom = mpGraphContextMenu->addAction("Reset Sample Zoom", this, &VisDisplaySpectrum::ResetSampleZoom);
    mpGraphContextMenu->addSeparator();

    mpActMoreFrequencies = mpGraphContextMenu->addAction("More Frequencies", this, &VisDisplaySpectrum::MoreFrequencies);
    mpActFewerFrequencies = mpGraphContextMenu->addAction("Fewer Frequencies", this, &VisDisplaySpectrum::FewerFrequencies);
    mpGraphContextMenu->addSeparator();

    mpActToggleColor = mpGraphContextMenu->addAction("Color Display", this, SLOT(ToggleColor()));
    mpActToggleColor->setCheckable(true);
    
    mpColorThemeMenu = new QMenu("Color Theme");
    QActionGroup* pColorThemeGroup = new QActionGroup(mpColorThemeMenu);
    mpActColorThemeAuto = mpColorThemeMenu->addAction("Auto", this, &VisDisplaySpectrum::ColorThemeAuto);
    mpActColorThemeAuto->setCheckable(true);
    pColorThemeGroup->addAction(mpActColorThemeAuto);
    mpActColorThemeLight = mpColorThemeMenu->addAction("Light", this, &VisDisplaySpectrum::ColorThemeLight);
    mpActColorThemeLight->setCheckable(true);
    pColorThemeGroup->addAction(mpActColorThemeLight);
    mpActColorThemeDark = mpColorThemeMenu->addAction("Dark", this, &VisDisplaySpectrum::ColorThemeDark);
    mpActColorThemeDark->setCheckable(true);
    mpActColorThemeDark->setChecked(true);
    pColorThemeGroup->addAction(mpActColorThemeDark);
    mpGraphContextMenu->addMenu(mpColorThemeMenu);

    mpGraphContextMenu->addSeparator();

    mpActToggleNumericValues = mpGraphContextMenu->addAction("Show Numeric Values", this, SLOT(ToggleNumericValues()));
    mpActToggleNumericValues->setCheckable(true);
    mpActToggleValueUnit = mpGraphContextMenu->addAction("Show Value Unit", this, &VisDisplaySpectrum::ToggleValueUnit);
    mpActToggleValueUnit->setCheckable(true);
    mpActToggleChannelList = mpGraphContextMenu->addAction("Show Channel List", this, SLOT(ToggleChannelList()));
    mpActToggleChannelList->setCheckable(true);
    mpActLargerFont = mpGraphContextMenu->addAction("Larger Font", this, SLOT(LargerFont()));
    mpActSmallerFont = mpGraphContextMenu->addAction("Smaller Font", this, SLOT(SmallerFont()));
}

void VisDisplaySpectrum::BuildChannelListContextMenu()
{
    QMenu* pMenu = new QMenu;
    
    pMenu->addAction("Activate all", this, &VisDisplaySpectrum::ChannelList_ActivateAll);
    pMenu->addAction("Deactivate all", this, &VisDisplaySpectrum::ChannelList_DeactivateAll);
    pMenu->addSeparator();

    mpActChannelList_Activate = pMenu->addAction("Activate", this, &VisDisplaySpectrum::ChannelList_Activate);
    mpActChannelList_Deactivate = pMenu->addAction("Deactivate", this, &VisDisplaySpectrum::ChannelList_Deactivate);

    mpChannelListContextMenu = pMenu;
}

void VisDisplaySpectrum::PopulateToolBar()
{
    mpToolBar->clear();
    mpToolBar->addAction(new ToolButtonAction("Frequency Scale Type", mpFrequencyScaleMenu, &mpFrequencyScaleButton));
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpActToggleAutoScale);
    mpToolBar->addAction(new ToolButtonAction("Value Scale Type", mpValueLogScaleMenu, &mpValueLogScaleButton));
    mpToolBar->addAction(mpActEnlargeSignal);
    mpToolBar->addAction(mpActReduceSignal);
    mpToolBar->addAction(mpActResetSignalScale);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpActMoreSamples);
    mpToolBar->addAction(mpActFewerSamples);
    mpToolBar->addAction(mpActResetSampleZoom);
    mpToolBar->addSeparator();
    mpToolBar->addAction(mpActMoreFrequencies);
    mpToolBar->addAction(mpActFewerFrequencies);
    mpToolBar->addSeparator();
    mpActColorTheme = new ToolButtonAction("Color Theme", mpColorThemeMenu, &mpColorThemeButton);
    mpToolBar->addAction(mpActColorTheme);

    LoadActionIcons();
}

void VisDisplaySpectrum::ShowToolBar(bool b)
{
    mpToolBar->setVisible(b);
}

void VisDisplaySpectrum::OnContextMenu(const QPoint &inP)
{
    if (mpChannelList && mpChannelList->isVisible() && mpChannelList->geometry().contains(inP)) {
        OnChannelListContextMenu(inP);
    }
    else {
        OnGraphContextMenu(inP);
    }
}

void VisDisplaySpectrum::OnGraphContextMenu(const QPoint &inP)
{
    mpGraphContextMenu->exec(this->mapToGlobal(inP));
}

void VisDisplaySpectrum::UpdateGraphActions()
{
    mpActEnlargeSignal->setEnabled(EnlargeSignal_Enabled());
    mpActReduceSignal->setEnabled(ReduceSignal_Enabled());
    mpActResetSignalScale->setEnabled(ResetSignalScale_Enabled());
    mpActToggleAutoScale->setEnabled(true);
    mpActToggleAutoScale->setChecked(ToggleAutoScale_Checked());

    mpActFewerSamples->setEnabled(FewerSamples_Enabled());
    mpActMoreSamples->setEnabled(MoreSamples_Enabled());
    mpActResetSampleZoom->setEnabled(ResetSampleZoom_Enabled());

    mpActFrequencyScaleLinear->setChecked(mDftType == linear);
    mpActFrequencyScaleConstantQ->setChecked(mDftType == constantQ);

    mpActFewerFrequencies->setEnabled(FewerFrequencies_Enabled());
    mpActMoreFrequencies->setEnabled(MoreFrequencies_Enabled());

    mpActToggleNumericValues->setChecked(ToggleNumericValues_Checked());

    mpActToggleColor->setEnabled(true);
    mpActToggleColor->setChecked(ToggleColor_Checked());

    mpActColorThemeAuto->setChecked(mColorTheme == auto_);
    mpActColorThemeLight->setChecked(mColorTheme == light_);
    mpActColorThemeDark->setChecked(mColorTheme == dark_);

    mpActToggleValueUnit->setEnabled(true);
    mpActToggleValueUnit->setChecked(ToggleValueUnit_Checked());

    mpActToggleChannelList->setEnabled(ToggleChannelList_Enabled());
    mpActToggleChannelList->setChecked(ToggleChannelList_Checked());

    mpActSmallerFont->setEnabled(SmallerFont_Enabled());
}

void VisDisplaySpectrum::OnChannelListContextMenu(const QPoint& inP)
{
    UpdateChannelListActions();
    mpChannelListContextMenu->exec(this->mapToGlobal(inP));
}

void VisDisplaySpectrum::UpdateChannelListActions()
{
    bool enabled = ChannelList_Enabled();
    for (auto pAction : {
        mpActChannelList_Activate,
        mpActChannelList_Deactivate,
        })
    {
        pAction->setEnabled(enabled);
    }
}

void VisDisplaySpectrum::ToggleNumericValues()
{
    mpSignalWidget->Display().SetNumericValuesVisible(!mpSignalWidget->Display().NumericValuesVisible());
    Visconfigs()[mVisID].Put(CfgID::ShowNumericValues, mpSignalWidget->Display().NumericValuesVisible(), UserDefined);
    UpdateGraphActions();
}

void VisDisplaySpectrum::ToggleValueUnit()
{
    mpSignalWidget->Display().SetValueUnitVisible(!mpSignalWidget->Display().ValueUnitVisible());
    Visconfigs()[mVisID].Put(CfgID::ShowValueUnit, mpSignalWidget->Display().ValueUnitVisible(), UserDefined);
    UpdateGraphActions();
}

bool VisDisplaySpectrum::ToggleNumericValues_Checked() const
{
    return mpSignalWidget->Display().NumericValuesVisible();
}

bool VisDisplaySpectrum::ToggleValueUnit_Checked() const
{
    return mpSignalWidget->Display().ValueUnitVisible();
}

bool VisDisplaySpectrum::ToggleChannelList_Enabled() const
{
    return mpChannelList;
}

bool VisDisplaySpectrum::ToggleChannelList_Checked() const
{
    return mpChannelList->isVisible();
}

void VisDisplaySpectrum::ToggleColor()
{
    mpSignalWidget->Display().SetColorDisplay(!mpSignalWidget->Display().ColorDisplay());
    UpdateGraphActions();
}

bool VisDisplaySpectrum::ToggleColor_Checked() const
{
    return mpSignalWidget->Display().ColorDisplay();
}

void VisDisplaySpectrum::FewerSamples()
{
    ++mUserZoom;
    ApplyUserZoom();
    UpdateGraphActions();
    InitializeAutoScale();
}

bool VisDisplaySpectrum::FewerSamples_Enabled() const
{
    if (mFixedSampleCount)
        return false;
    float nominalSamples = mConfigDisplaySamples * ::pow(2, -mUserZoom - 1);
    return mUserZoom < kMaxUserScaling && nominalSamples >= 2 * mSignalElements;
}

void VisDisplaySpectrum::MoreSamples()
{
    --mUserZoom;
    ApplyUserZoom();
    UpdateGraphActions();
    InitializeAutoScale();
}

bool VisDisplaySpectrum::MoreSamples_Enabled() const
{
    if (mFixedSampleCount)
        return false;
    return mUserZoom > -kMaxUserScaling;
}

void VisDisplaySpectrum::ResetSampleZoom()
{
    mUserZoom = 0;
    ApplyUserZoom();
    UpdateGraphActions();
}

bool VisDisplaySpectrum::ResetSampleZoom_Enabled() const
{
    if (mFixedSampleCount)
        return false;
    return mUserZoom != 0;
}

void VisDisplaySpectrum::FewerFrequencies()
{
    auto& display = mpSignalWidget->Display();
    // Round down to the nearest power of 2.
    int n = display.DisplayGroups(), roundedN = 1;
    while (n >>= 1)
        roundedN <<= 1;

    int newGroups = roundedN;
    if (newGroups == display.DisplayGroups())
        newGroups /= 2;
    display.SetDisplayGroups(newGroups);
    UpdateGraphActions();
}

bool VisDisplaySpectrum::FewerFrequencies_Enabled() const
{
    return mpSignalWidget->Display().DisplayGroups() > 1;
}

void VisDisplaySpectrum::MoreFrequencies()
{
    auto& display = mpSignalWidget->Display();
    int maxDisplayGroups = mSpectrumSize,
        newDisplayGroups = std::min(maxDisplayGroups, display.DisplayGroups() * 2);
    display.SetDisplayGroups(newDisplayGroups);
    UpdateGraphActions();
}

bool VisDisplaySpectrum::MoreFrequencies_Enabled() const
{
    return mpSignalWidget->Display().DisplayGroups() < mSpectrumSize;
}

double VisDisplaySpectrum::UnitToValue(const std::string& inUnit) const
{
    PhysicalUnit u;
    u.SetGainWithSymbol("1" + inUnit);
    return u.Symbol() == "s" ? u.Gain() : 0;
}

void VisDisplaySpectrum::LargerFont()
{
    int size = Round(mpSignalWidget->Display().FontSize() * 1.1);
    if (size == mpSignalWidget->Display().FontSize())
        ++size;
    mpSignalWidget->Display().SetFontSize(size);
    Visconfigs()[mVisID].Put(CfgID::FontSize, mpSignalWidget->Display().FontSize(), UserDefined);
    UpdateGraphActions();
}

void VisDisplaySpectrum::SmallerFont()
{
    int size = Round(mpSignalWidget->Display().FontSize() / 1.1);
    if (size == mpSignalWidget->Display().FontSize())
        --size;
    mpSignalWidget->Display().SetFontSize(size);
    Visconfigs()[mVisID].Put(CfgID::FontSize, mpSignalWidget->Display().FontSize(), UserDefined);
    UpdateGraphActions();
}

void VisDisplaySpectrum::ToggleChannelList()
{
    if (mpChannelList)
        mpChannelList->setVisible(!mpChannelList->isVisible());
    Visconfigs()[mVisID].Put(CfgID::ShowChannelList, mpChannelList && mpChannelList->isVisible(), UserDefined);
    UpdateGraphActions();
}

bool VisDisplaySpectrum::SmallerFont_Enabled() const
{
    return mpSignalWidget->Display().FontSize() > 1;
}

int VisDisplaySpectrum::DisplaySamples() const
{
    return mpSignalWidget->Display().NumSamples();
}

void VisDisplaySpectrum::SetDisplaySamples(int inSamples)
{
    if (inSamples != DisplaySamples())
    {
        double unitsPerSample = UnitsPerSample();
        mpSignalWidget->Display().SetNumSamples(inSamples);
        SetUnitsPerSample(unitsPerSample);
    }
}

double VisDisplaySpectrum::UnitsPerSample() const
{
    return mpSignalWidget->Display().UnitsPerSample();
}

void VisDisplaySpectrum::SetUnitsPerSample(double inUps)
{
    mpSignalWidget->Display().SetUnitsPerSample(inUps);
}

void VisDisplaySpectrum::SetValueScale(float inScale, bool inWriteToConfig)
{
    mpSignalWidget->Display().SetValueLogScale(inScale);
    SyncLogScaleMenu(inScale, mpValueLogScaleMenu);
    if (inWriteToConfig)
        Visconfigs()[mVisID].Put(CfgID::YAxisLogScale, inScale, UserDefined);
    UpdateGraphActions();
}

void VisDisplaySpectrum::SetValueScaleLinear()
{
    SetValueScale(0, true);
}

void VisDisplaySpectrum::SetValueScaleLog10()
{
    SetValueScale(10, true);
}

void VisDisplaySpectrum::SetValueScaleLog20()
{
    SetValueScale(20, true);
}

void VisDisplaySpectrum::SetValueScaleLog100()
{
    SetValueScale(100, true);
}

void VisDisplaySpectrum::SetValueScaleCustom()
{
    QInputDialog dialog(this);
    double scale = mpSignalWidget->Display().SampleLogScale();
    dialog.setDoubleValue(scale);
    dialog.setDoubleRange(0, 1e4);
    dialog.setWindowTitle("Choose a custom log scale factor");
    if (dialog.exec()) {
        SetValueScale(dialog.doubleValue(), true);
    }
}

QAction* VisDisplaySpectrum::SyncLogScaleMenu(float inScale, QMenu* inpMenu)
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

void VisDisplaySpectrum::SetDftUniform()
{
    mDftType = linear;
    InitializeDftConfig();
    SaveDftConfig();
}

void VisDisplaySpectrum::SetDftConstantQ()
{
    mDftType = constantQ;
    InitializeDftConfig();
    SaveDftConfig();
}

void VisDisplaySpectrum::InitializeDftConfig()
{
    std::unique_lock<std::mutex> lock(mDftMutex);
    // For performance reasons, we compute the frequency spectrum between 0 and 100Hz only.
    // This allows us to decimate the signal down to a target frequency of about 200Hz.
    constexpr double targetFrequency = 200; // Hz
    int srDecimation = std::floor(mInputSamplingRate / targetFrequency);
    if (srDecimation < 1)
        srDecimation = 1;

    mDecimationLp.LPCorner(0.5 / srDecimation);
    mDecimationFilter.Decimation(srDecimation);

    mDftSpectraDecimation = mDftDecimation / srDecimation;
    if (mDftSpectraDecimation < 1)
        mDftSpectraDecimation = 1;

    switch (mDftType) {
    case linear:
        mDftConfig = ContinuousDft::Config::CreateLinear(mDftNumBins);
        break;
    default:
        mDftConfig = ContinuousDft::Config::CreateConstantQ(mDftNumBins);
    }
    mDftConfig.Flip();

    auto& display = mpSignalWidget->Display();
    display.SetChannelGroupSize(1);
    display.SetDisplayGroups(mDftConfig.BinCount());
    LabelList frequencyLabels = BuildFrequencyLabels();
    display.SetChannelLabels(frequencyLabels);
    display.SetChannelLabelsVisible(true);

    mDftObserver.Clear();
}

void VisDisplaySpectrum::SaveDftConfig()
{
    auto& config = Visconfigs()[mVisID];

    config.Put(CfgID::DftType, mDftType == linear ? CfgID::DftLinear : CfgID::DftConstantQ, UserDefined);
    config.Put(CfgID::DftNumBins, mDftNumBins, UserDefined);
    config.Put(CfgID::DftDecimate, mDftDecimation, UserDefined);
}

void VisDisplaySpectrum::InitializeDft()
{
    mDftObserver = ContinuousDft::SignalObserver<ContinuousDftNumericType>(mNumChannels, mDftConfig);
    mDftObserver.SetOutputDecimation(mDftSpectraDecimation);
    for (const auto& entry : mChannelsVisible) {
        // mChannelMap provides Channel Name->Channel Index
        auto i = mChannelMap.find(entry.first);
        if (i != mChannelMap.end() && i->second < mDftObserver.BankCount()) {
            mDftObserver.SetChannelActive(i->second, entry.second);
        }
    }
}

LabelList VisDisplaySpectrum::BuildFrequencyLabels() const
{
    std::vector<double> frequencies;
    for (int i = 0; i < mDftConfig.BinCount(); ++i) {
        frequencies.push_back(mDftConfig.Frequency(i));
    }
    // Note that the list of frequencies is bottom up.
    std::reverse(frequencies.begin(), frequencies.end());

    PhysicalUnit u;
    u.SetGain(mInputSamplingRate / mDecimationFilter.Decimation()).SetSymbol("Hz");
    auto physicalValues = u.RawToPhysicalTable(frequencies, 1, 3);

    LabelList labels;
    int i = physicalValues.size();
    for (auto physicalValue : physicalValues) {
        labels.push_back(Label(--i, physicalValue));
    }
    return labels;
}

void VisDisplaySpectrum::BuildChannelList()
{
    Assert(mpChannelList == nullptr);
    mpChannelList = new QListWidget;
    mpChannelList->setFrameStyle(QFrame::NoFrame);
    mpChannelList->setFixedWidth(0);
    mpChannelList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mpChannelList->setFocusPolicy(Qt::NoFocus);
    connect(mpChannelList, &QListWidget::itemChanged, this, &VisDisplaySpectrum::OnChannelListItemChanged);
    connect(mpChannelList, &QListWidget::itemSelectionChanged, this, &VisDisplaySpectrum::OnChannelListItemSelectionChanged);
}

void VisDisplaySpectrum::UpdateChannelList(const LabelList& labels)
{
    if (mpChannelList)
    {
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
        }
        int width = mpChannelList->sizeHintForColumn(0);
        width += mpChannelList->fontMetrics().horizontalAdvance("  ");
        width += qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        mpChannelList->setFixedWidth(width);

        mChannelsSelected.clear();
        mChannelsSelected.resize(labels.Size());
    }
}

bool VisDisplaySpectrum::ChannelList_Enabled() const
{
    return mpChannelList && mpChannelList->selectedItems().count() > 0;
}

bool VisDisplaySpectrum::EnlargeSignal_Enabled() const
{
    if (mAutoScaleFilter.enabled)
        return false;
    return mUserScaling < kMaxUserScaling;
}

bool VisDisplaySpectrum::ReduceSignal_Enabled() const
{
    if (mAutoScaleFilter.enabled)
        return false;
    return mUserScaling > -kMaxUserScaling;
}

bool VisDisplaySpectrum::ResetSignalScale_Enabled() const
{
    if (mAutoScaleFilter.enabled)
        return false;
    return mUserScaling != 0;
}

void VisDisplaySpectrum::ChannelList_Activate()
{
    if (mpChannelList)
    {
        auto selected = mpChannelList->selectedItems();
        for (const auto pItem : selected)
            pItem->setCheckState(Qt::Checked);
    }
}

void VisDisplaySpectrum::ChannelList_Deactivate()
{
    if (mpChannelList)
    {
        auto selected = mpChannelList->selectedItems();
        for (const auto pItem : selected)
            pItem->setCheckState(Qt::Unchecked);
    }
}

void VisDisplaySpectrum::EnlargeSignal()
{
    ++mUserScaling;
    ApplyUserScaling();
    UpdateGraphActions();
}

void VisDisplaySpectrum::ReduceSignal()
{
    --mUserScaling;
    ApplyUserScaling();
    UpdateGraphActions();
}

void VisDisplaySpectrum::ResetSignalScale()
{
    mUserScaling = 0;
    ApplyUserScaling();
}

void VisDisplaySpectrum::ToggleAutoScale()
{
    mAutoScaleFilter.enabled = !mAutoScaleFilter.enabled;
    if (!mAutoScaleFilter.enabled) {
        ApplyUserScaling();
    }
    Visconfigs()[mVisID].Put(CfgID::AutoScale, mAutoScaleFilter.enabled, UserDefined);
    UpdateGraphActions();
}

bool VisDisplaySpectrum::ToggleAutoScale_Checked() const
{
    return mAutoScaleFilter.enabled;
}

void VisDisplaySpectrum::OnChannelListItemSelectionChanged()
{
    Assert(mChannelsSelected.size() >= mpChannelList->count());
    for (int row = 0; row < mpChannelList->count(); ++row) {
        bool selected = mpChannelList->item(row)->isSelected();
        mChannelsSelected[row] = selected;
    }
}

void VisDisplaySpectrum::OnChannelListItemChanged(QListWidgetItem* pItem)
{
    auto var = pItem->data(Qt::UserRole);
    if (var.isValid())
    {
        double index = var.toDouble();
        bool checked = (pItem->checkState() == Qt::Checked);
        mDftObserver.SetChannelActive(index, checked);
        mChannelsVisible[pItem->text().toUtf8().constData()] = checked;
        Visconfigs()[mVisID].Put(CfgID::ChannelsVisible, mChannelsVisible, UserDefined);
        UpdateGraphActions();
    }
}

void VisDisplaySpectrum::ChannelList_DeactivateAll()
{
    if (mpChannelList)
    {
        for (int i = 0; i < mpChannelList->count(); ++i)
            mpChannelList->item(i)->setCheckState(Qt::Unchecked);
    }
}

void VisDisplaySpectrum::ChannelList_ActivateAll()
{
    if (mpChannelList)
    {
        for (int i = 0; i < mpChannelList->count(); ++i)
            mpChannelList->item(i)->setCheckState(Qt::Checked);
    }
}

void VisDisplaySpectrum::ColorThemeAuto()
{
    mColorTheme = auto_;
    Visconfigs()[mVisID].Put(CfgID::ColorTheme, mColorTheme, UserDefined);
    ApplyColorTheme();
    UpdateGraphActions();
}

void VisDisplaySpectrum::ColorThemeLight()
{
    mColorTheme = light_;
    Visconfigs()[mVisID].Put(CfgID::ColorTheme, mColorTheme, UserDefined);
    ApplyColorTheme();
    UpdateGraphActions();
}

void VisDisplaySpectrum::ColorThemeDark()
{
    mColorTheme = dark_;
    Visconfigs()[mVisID].Put(CfgID::ColorTheme, mColorTheme, UserDefined);
    ApplyColorTheme();
    UpdateGraphActions();
}

void VisDisplaySpectrum::SetChannelSelected(int ch, bool selected)
{
    Assert(ch < mChannelsSelected.size());
    mChannelsSelected[ch] = selected;
    UpdateChannelSelection();
}

bool VisDisplaySpectrum::ToggleChannelSelected(int ch)
{
    Assert(ch < mChannelsSelected.size());
    mChannelsSelected[ch] = !mChannelsSelected[ch];
    UpdateChannelSelection();
    return mChannelsSelected[ch];
}

void VisDisplaySpectrum::UpdateChannelSelection()
{
    if (mpChannelList)
    {
        bool oldState = mpChannelList->blockSignals(true);
        for (int row = 0; row < mpChannelList->count(); ++row) {
            mpChannelList->item(row)->setSelected(mChannelsSelected[row]);
        }
        mpChannelList->blockSignals(oldState);
    }
}

void VisDisplaySpectrum::SelectAllChannels()
{
    for (int i = 0; i < mChannelsSelected.size(); ++i) {
        mChannelsSelected[i] = true;
    }
    UpdateChannelSelection();
}

void VisDisplaySpectrum::ClearChannelSelection()
{
    for (int i = 0; i < mChannelsSelected.size(); ++i) {
        mChannelsSelected[i] = false;
    }
    UpdateChannelSelection();
}

void VisDisplaySpectrum::ApplyUserScaling()
{
    double factor = std::pow(2.0, -mUserScaling);
    mpSignalWidget->Display().SetMinValue(factor * mMinValue);
    mpSignalWidget->Display().SetMaxValue(factor * mMaxValue);

    auto& config = Visconfigs()[mVisID];
    config.Put(CfgID::UserScaling, mUserScaling, UserDefined);
}

void VisDisplaySpectrum::ApplyUserZoom()
{
    float newNominalSamples = mConfigDisplaySamples * ::pow(2, -mUserZoom);
    if (newNominalSamples >= 2 * mSignalElements)
        SetDisplaySamples(newNominalSamples);
    Visconfigs()[mVisID].Put(CfgID::UserSampleZoom, mUserZoom, UserDefined);
}

void VisDisplaySpectrum::ApplyColorTheme()
{
    bool lightMode = (mColorTheme == light_);
    if (mColorTheme == auto_)
        lightMode = !IsPaletteDark();
    mpSignalWidget->Display().SetLightMode(lightMode);
}

void VisDisplaySpectrum::InitializeAutoScale()
{
    // Choose auto scale time constant such that it matches the number of samples in the display.
    mAutoScaleFilter.decayFactor = std::exp(-1.0/DisplaySamples());
}

void VisDisplaySpectrum::ObserveAutoScale(const GenericSignal& spectrum)
{
    double maxVal = 0;
    int count = spectrum.Channels() * spectrum.Elements();
    auto pSpectrum = spectrum.ConstData();
    for (int i = 0; i < count; ++i) {
        maxVal = std::max(maxVal, pSpectrum[i]);
    }
    mAutoScaleFilter.count *= mAutoScaleFilter.decayFactor;
    mAutoScaleFilter.count += 1;
    mAutoScaleFilter.sum *= mAutoScaleFilter.decayFactor;
    mAutoScaleFilter.sum += maxVal;
}

void VisDisplaySpectrum::ApplyAutoScale()
{
    if (mAutoScaleFilter.enabled) {
        float meanMaxValue = mAutoScaleFilter.sum / mAutoScaleFilter.count;
        SignalDisplay& display = mpSignalWidget->Display();
        display.SetMaxValue(meanMaxValue);
        display.SetMinValue(0);
    }
}

void VisDisplaySpectrum::hideEvent(QHideEvent *iopEvent)
{
    VisDisplayBase::hideEvent(iopEvent);
    iopEvent->accept();
}

void VisDisplaySpectrum::moveEvent(QMoveEvent *iopEvent)
{
    VisDisplayBase::moveEvent(iopEvent);
    iopEvent->accept();
}

void VisDisplaySpectrum::resizeEvent(QResizeEvent *iopEvent)
{
    VisDisplayBase::resizeEvent(iopEvent);
    iopEvent->accept();
}

void VisDisplaySpectrum::keyReleaseEvent(QKeyEvent *iopEvent)
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
        if (FewerFrequencies_Enabled())
            FewerFrequencies();
        break;
    case Qt::Key_Greater:
    case Qt::Key_Period:
        if (MoreFrequencies_Enabled())
            MoreFrequencies();
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

void VisDisplaySpectrum::wheelEvent(QWheelEvent* iopEvent)
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

void VisDisplaySpectrum::changeEvent(QEvent *iopEvent)
{
    if (iopEvent->type() == QEvent::PaletteChange) {
        PopulateToolBar();
        ApplyColorTheme();
    }
    VisDisplayBase::changeEvent(iopEvent);
}

