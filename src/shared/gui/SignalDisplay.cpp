////////////////////////////////////////////////////////////////////////////////
// $Id: SignalDisplay.cpp 9547 2026-09-09 10:00:01Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A SignalDisplay class that renders GenericSignal data into a
//   given DisplayContext.
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
#include "SignalDisplay.h"

#include "Exception.h"
#include "FastConv.h"
#include "Tree.h"
#include "Debugging.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "QtWidgetProxy.h"

#include <QPainter>
#include <QWidget>
#include <QRgb>
#include <QMetaObject>

#undef TEST_UPDATE_RGN

////////////////////////////////////////////////////////////////////////////////
const RGBColor SignalDisplay::cAxisColorDefault = RGBColor::Aqua;
const RGBColor SignalDisplay::cChannelColorsDefault[] = {RGBColor::White, RGBColor::White,  RGBColor::White,
                                                         RGBColor::White, RGBColor::Yellow, ColorList::End};

SignalDisplay::SignalDisplay()
    : mDataWidth(0), mDataHeight(0), mDataLeft(cLabelWidth), mLabelWidth(cLabelWidth), mMarkerHeight(0),
      mDisplayMode(polyline), mShowCursor(false), mWrapAround(false), mTimeLabels(false), mShowNumericValues(false),
      mShowBaselines(false), mShowGrid(false), mShowChannelLabels(false), mShowValueUnit(false), mDisplayColors(true), mLightMode(false),
      mSampleLogScale(0), mValueLogScale(0),
      mNumSamples(cNumSamplesDefault), mCursorPosition(0), mNumDisplayGroups(0), mMaxDisplayGroups(0),
      mTopGroup(0), mChannelGroupSize(1), mTrivialChannelGroups(true),
      mMarkerChannels(0), mTickLength(4), mCursorWidth(3),
      mCursorColor(RGBColor::Yellow), mSelectionColor(RGBColor::NullColor),
      mFontSize(11), mNumericValueWidth(0), mMinValue(cMinValueDefault), mMaxValue(cMaxValueDefault),
      mAxisColor(cAxisColorDefault), mChannelColors(cChannelColorsDefault), mpParent(nullptr), mpSignalPoints(nullptr), mSignalPointsCount(0),
      mSymbolWidth(cLabelWidth), mXTicksVisible(true), mYTicksVisible(true), mFastScrolling(false), mAutoDecimationMaxLinesPerPixel(0)
{
}

SignalDisplay::~SignalDisplay()
{
    delete[] mpSignalPoints;
    delete mpParentWidgetProxy;
}

namespace
{

void SetUnitFromString(const std::string &s, PhysicalUnit &u)
{
    if (::atof(s.c_str()))
        u.SetGainWithSymbol(s);
    else
        u.SetSymbol(s);
}

float Dither(float f)
{
    // modf() returns signed results, which we don't want
    float intpart = ::floor(f),
          fracpart = f - intpart;
    if (::rand() < static_cast<float>(RAND_MAX) * fracpart)
        intpart += 1;
    return intpart;
}

struct MarkerDef
{
    std::string label;
    QColor color;
    Qt::PenStyle style;
    float width;
};

Qt::PenStyle ParsePenStyle(const std::string& style)
{
    if (style == "-")
        return Qt::SolidLine;
    else if (style == "." || style == "..")
        return Qt::DotLine;
    else if (style == "--")
        return Qt::DashLine;
    else if (style == ".-" || style == "-.")
        return Qt::DashDotLine;
    else if (style == ".-." || style == "-.." || style == "..-")
        return Qt::DashDotDotLine;
    return Qt::NoPen;
}

MarkerDef ParseMarkerDef(const std::string& markerdef, bool inverted)
{
    auto def = StringUtils::Split("|", markerdef);
    Assert(!def.empty());
    // https://www.w3.org/TR/SVG/types.html#ColorKeywords
    if (def.size() < 2)
        def.push_back("");
    if (def[1].empty())
        def[1] = "white";
    if (def.size() < 3)
        def.push_back("");
    if (def.size() < 4)
        def.push_back("1");
    if (inverted)
    {
        if (def[1] == "white")
            def[1] = "black";
        else if (def[1] == "black")
            def[1] = "white";
    }
    QColor color(def[1].c_str());
    Qt::PenStyle style = ParsePenStyle(def[2]);
    float width = ::atof(def[3].c_str());
    return MarkerDef{def[0], color, style, width};
}

template<class T> std::vector<T> MergeSortedLists(const std::vector<T>& list1, const std::vector<T>& list2)
{
    std::vector<T> list3;
    list3.reserve(list1.size() + list2.size());
    int i = 0, j = 0;
    while (i < list1.size() || j < list2.size()) {
        if (i < list1.size() && j < list2.size()) {
            if (list1[i] < list2[j]) {
                list3.push_back(list1[i++]);
            }
            else {
                list3.push_back(list2[j++]);
            }
        }
        else if (i < list1.size()) {
            list3.push_back(list1[i++]);
        }
        else if (j < list2.size()) {
            list3.push_back(list2[j++]);
        }
    }
    return list3;
}

float ForwardLogTransform(float x, float xmax, float scale)
{
    float f = ::log(1 + scale * ::fabs(x) / xmax) / ::log(1 + scale) * xmax;
    if (x < 0)
        return -f;
    return f;
}

float BackwardLogTransform(float x, float xmax, float scale)
{
    return (::exp(x / xmax * ::log(1 + scale)) - 1) * xmax / scale;
}

} // namespace

float SignalDisplay::SampleLeft(int s) const
{
    float x = 0;
    switch (mDisplayMode) {
        case polyline:
            x = (mNumSamples > 1 ? (s * (mDataWidth - 1)) / float(mNumSamples - 1) : 0);
            break;
        case field2d:
            x = (mNumSamples ? (s * mDataWidth) / float(mNumSamples) : 0);
            break;
        default:
            ;
    }
    if (mSampleLogScale > 0)
        x = ForwardLogTransform(x, mDataWidth, mSampleLogScale);
    x += mDataLeft;
    return x;
}
float SignalDisplay::SampleRight(int s) const
{
    return SampleLeft(s + 1);
}
float SignalDisplay::PosToSample(float p) const
{
    float s = 0;
    p -= mDataLeft;
    if (mSampleLogScale > 0)
        p = BackwardLogTransform(p, mDataWidth, mSampleLogScale);

    switch (mDisplayMode) {
        case polyline:
            s = mDataWidth > 1 ? (p * (mNumSamples - 1)) / float(mDataWidth - 1) : 0;
            break;
        case field2d:
            s = mDataWidth ? (p * mNumSamples) / float(mDataWidth) : 0;
            break;
        default:
            ;
    }
    return s;
}

SignalDisplay &SignalDisplay::SetChannelUnit(const std::string &s)
{
    SetUnitFromString(s, mChannelUnit);
    InvalidateWidget();
    return *this;
}

SignalDisplay& SignalDisplay::SetMinValues(const std::vector<double>& v)
{
    if (v != mMinValues)
    {
        mMinValues = v;
        auto it = std::min_element(mMinValues.begin(), mMinValues.end());
        if (it != mMinValues.end())
            mMinValue = *it;
        return Invalidate();
    }
    return *this;
}

const std::vector<double>& SignalDisplay::MinValues() const
{
    return mMinValues;
}

SignalDisplay& SignalDisplay::SetMaxValues(const std::vector<double>& v)
{
    if (v != mMaxValues)
    {
        mMaxValues = v;
        auto it = std::max_element(mMaxValues.begin(), mMaxValues.end());
        if (it != mMaxValues.end())
            mMaxValue = *it;
        return Invalidate();
    }
    return *this;
}

const std::vector<double>& SignalDisplay::MaxValues() const
{
    return mMaxValues;
}

SignalDisplay &SignalDisplay::SetSampleUnit(const std::string &s)
{
    SetUnitFromString(s, mSampleUnit);
    mTimeLabels = (mSampleUnit.Symbol() == ":s");
    InvalidateWidget();
    return *this;
}

SignalDisplay &SignalDisplay::SetValueUnit(const std::string &s)
{
    SetUnitFromString(s, mValueUnit);
    InvalidateWidget();
    return *this;
}

SignalDisplay& SignalDisplay::SetValueUnits(const ValueList<std::string>& units)
{
    mValueUnits.clear();
    mValueUnits.reserve(units.size());
    for (const auto& unit : units)
    {
        PhysicalUnit u;
        SetUnitFromString(unit, u);
        mValueUnits.push_back(u);
    }
    InvalidateWidget();
    return *this;
}

ValueList<std::string> SignalDisplay::ValueUnits() const
{
    ValueList<std::string> units;
    units.reserve(mValueUnits.size());
    for (const auto& unit : mValueUnits)
        units.push_back(unit.GainWithSymbol());
    return units;
}

SignalDisplay& SignalDisplay::SetChannelVisible(int ch, bool visible)
{
    if (ch < 0)
        return *this;

    if (ch >= mChannelVisible.size())
        mChannelVisible.resize(ch + 1, true);

    if (visible != mChannelVisible[ch])
    {
        mChannelVisible[ch] = visible;
        RebuildChannelGroups();
        DetermineMaxDisplayGroups();
        AdjustDisplayGroups();
        Invalidate();
    }
    return *this;
}

bool SignalDisplay::ChannelVisible(int inCh) const
{
    if (inCh < 0)
        return false;
    if (inCh >= mChannelVisible.size())
        return true;
    return mChannelVisible[inCh];
}

SignalDisplay& SignalDisplay::SetChannelsVisible(const ValueList<bool>& channelsVisible)
{
    if (channelsVisible != mChannelVisible)
    {
        mChannelVisible = channelsVisible;
        RebuildChannelGroups();
        DetermineMaxDisplayGroups();
        AdjustDisplayGroups();
        Invalidate();
    }
    return *this;
}

ValueList<bool> SignalDisplay::ChannelsVisible() const
{
    ValueList<bool> list;
    list.reserve(mChannelVisible.size());
    for (bool b : mChannelVisible)
        list.push_back(b);
    return list;
}

SignalDisplay& SignalDisplay::SetChannelSelected(const std::vector<bool>& channelSelected)
{
    mChannelSelected = channelSelected;
    return Invalidate();
}

std::vector<bool> SignalDisplay::ChannelSelected() const
{
    return mChannelSelected;
}

std::string SignalDisplay::ValueColors() const
{
    Tree tree;
    int i = 0;
    for (const auto &pair : mValueColors)
    {
        tree[i][0].Data() << pair.first;
        Tree &c = tree[i][1];
        c[0].Data() << pair.second.R();
        c[1].Data() << pair.second.G();
        c[2].Data() << pair.second.B();
        ++i;
    }
    return tree.ToString();
}

SignalDisplay& SignalDisplay::SetSelectionColor(RGBColor c)
{
    mSelectionColor = c;
    return *this;
}

RGBColor SignalDisplay::SelectionColor() const
{
    return mSelectionColor;
}

SignalDisplay &SignalDisplay::SetValueColors(std::string s)
{
    mValueColors.clear();
    Tree tree;
    if (tree.Parse(s))
    {
        for (const auto &entry : tree.Children())
        {
            double value;
            entry[0].Data() >> value;
            RGBColor color;
            if (entry[1].Size() == 1)
            {
                entry[1].Data() >> color;
            }
            else
            {
                int rgb[3] = { 0 };
                for (int i = 0; i < 3; ++i)
                    entry[1][i].Data() >> rgb[i];
                color = RGBColor(rgb[0], rgb[1], rgb[2]);
            }
            mValueColors.push_back(std::make_pair(value, color));
        }
        std::sort(mValueColors.begin(), mValueColors.end());
    }
    return Invalidate();
}

SignalDisplay &SignalDisplay::SetContext(const GUI::DrawContext &dc)
{
    mDisplayRect = QRect(ToInt(dc.rect.left), ToInt(dc.rect.top), ToInt(dc.rect.right - dc.rect.left),
                         ToInt(dc.rect.bottom - dc.rect.top));
    mDisplayRgn = mDisplayRect;
    if (mImageBuffer.size() != mDisplayRect.size())
    {
        mImageBuffer = QImage(mDisplayRect.size(), QImage::Format_ARGB32_Premultiplied);
        InvalidateBuffer();
    }
    if (dc.device != mpParent) {
        mpParent = dc.device;
        delete mpParentWidgetProxy;
        mpParentWidgetProxy = nullptr;
        auto pWidget = dynamic_cast<QWidget*>(mpParent);
        if (pWidget) {
            mpParentWidgetProxy = new QtWidgetProxy(pWidget);
        }
    }
    InvalidateWidget();
    return *this;
}

void SignalDisplay::InvalidateBuffer(const void* inRegion)
{
    std::unique_lock<std::mutex> lock(mBufferMutex);
    const QRegion *pRegion = static_cast<const QRegion*>(inRegion);
    if (!pRegion)
        mInvalidBufferRgn = mDisplayRect;
    else
        mInvalidBufferRgn += *pRegion;
}

void SignalDisplay::InvalidateWidget(const void *inRegion)
{
    if (mpParentWidgetProxy)
    {
        if (!inRegion) {
            QRegion region = mDisplayRect;
            bool success = QMetaObject::invokeMethod(mpParentWidgetProxy, "UpdateRegion", Qt::AutoConnection, Q_ARG(QRegion, region));
            Assert(success);
        }
        else {
            QRegion region = *static_cast<const QRegion*>(inRegion);
            bool success = QMetaObject::invokeMethod(mpParentWidgetProxy, "UpdateRegion", Qt::AutoConnection, Q_ARG(QRegion, region));
            Assert(success);
        }
    }
}

SignalDisplay& SignalDisplay::Invalidate(const void* inRegion)
{
    InvalidateBuffer(inRegion);
    InvalidateWidget(inRegion);
    return *this;
}

void SignalDisplay::DetermineMaxDisplayGroups()
{
    int newNumDisplayGroups = mChannelGroups.size();
    if (mMaxDisplayGroups == 0)
        mMaxDisplayGroups = std::min<int>(newNumDisplayGroups, cInitialMaxDisplayGroups);
    else if (newNumDisplayGroups < mMaxDisplayGroups)
        mMaxDisplayGroups = newNumDisplayGroups;
}

void SignalDisplay::AdaptTo(const GenericSignal &inSignal)
{
    // Any changes in signal size that we must react to?
    bool reconfigure = false;
    if (inSignal.Elements() > mNumSamples)
    {
        std::unique_lock<std::mutex> lock(mDataMutex);
        SetNumSamples(inSignal.Elements());
        reconfigure = true;
    }
    if (inSignal.Channels() != mData.Channels())
    {
        std::unique_lock<std::mutex> lock(mDataMutex);
        mChannelVisible.resize(inSignal.Channels() - mMarkerChannels, true);
        RebuildChannelGroups();
        DetermineMaxDisplayGroups();
        reconfigure = true;
    }
    if (reconfigure)
    {
        std::unique_lock<std::mutex> lock(mDataMutex);
        mData = GenericSignal(SignalProperties(inSignal.Channels(), mNumSamples), GenericSignal::NaN);
        SetDisplayGroups(mMaxDisplayGroups);
        SyncLabelWidth();
        mCursorPosition = 0;
        lock.unlock();
        Invalidate();
    }
}

SignalDisplay &SignalDisplay::WrapForward(const GenericSignal &inSignal)
{
    if (mpParent == nullptr)
        return *this;

    AdaptTo(inSignal);

    {
        std::unique_lock<std::mutex> lock(mDataMutex);
        mShowCursor = (inSignal.Elements() < mNumSamples);

        const auto* pInput = inSignal.ConstData();
        auto* pOutput = mData.MutableData(GenericSignal::intentRetainContents);
        for (int ch = 0; ch < inSignal.Channels(); ++ch) {
            const auto* pInputChannel = pInput + ch * inSignal.Elements();
            auto* pOutputChannel = pOutput + ch * mData.Elements();
            int begin = mCursorPosition, end = mCursorPosition + inSignal.Elements();
            while (end > mData.Elements())
                end -= mData.Elements();

            if (begin < end) {
                ::memcpy(pOutputChannel + begin, pInputChannel, (end - begin) * sizeof(GenericSignal::ValueType));
            }
            else {
                int count1 = mData.Elements() - begin;
                int count2 = inSignal.Elements() - count1;
                ::memcpy(pOutputChannel + begin, pInputChannel, count1 * sizeof(GenericSignal::ValueType));
                ::memcpy(pOutputChannel, pInputChannel + count1, count2 * sizeof(GenericSignal::ValueType));
            }
        }
    }

    SyncGraphics();

    int firstInvalidSample = mCursorPosition, firstValidSample = mCursorPosition + inSignal.Elements();
    bool wrappingAround = false;
    if (mNumSamples > 0)
    {
        wrappingAround = (firstValidSample / mNumSamples > 0);
        mWrapAround |= wrappingAround;
        wrappingAround |= bool(mCursorPosition == 0);
        mCursorPosition = firstValidSample % mNumSamples;
    }

    int firstInvalidPixel = std::floor(SampleLeft(firstInvalidSample));
    int firstValidPixel = std::ceil(SampleLeft(firstValidSample));

    QWidget *pWindow = dynamic_cast<QWidget *>(mpParent);
    if (pWindow)
    {
        QRegion invalidRgn;

        invalidRgn += mPrevCursorRect;
        invalidRgn += CursorRect(mCursorPosition);

        QRect invalidRect = mDataRect;

        // The non-wrapped area.
        invalidRect.setLeft(std::max<int>(firstInvalidPixel, mDataRect.left()) - mSymbolWidth / 2);
        invalidRect.setRight(std::min<int>(firstValidPixel, mDataRect.right()));
        if (invalidRect.left() < invalidRect.right())
            invalidRgn += invalidRect;

        // The area wrapped around the left edge.
        invalidRect.setLeft(std::max<int>(firstInvalidPixel + mDataWidth, mDataRect.left()) - mSymbolWidth / 2);
        invalidRect.setRight(std::min<int>(firstValidPixel + mDataWidth, mDataRect.right()));
        if (invalidRect.left() < invalidRect.right())
            invalidRgn += invalidRect;

        // The area wrapped around the right edge.
        invalidRect.setLeft(std::max<int>(firstInvalidPixel - mDataWidth, mDataRect.left()) - mSymbolWidth / 2);
        invalidRect.setRight(std::min<int>(firstValidPixel - mDataWidth, mDataRect.right()));
        if (invalidRect.left() < invalidRect.right())
            invalidRgn += invalidRect;

        if (mShowNumericValues)
        {
            invalidRect.setLeft(NumericValuesLeft());
            invalidRect.setRight(NumericValuesRight());
            invalidRgn += invalidRect;
        }
        Invalidate(&invalidRgn);
    }
    return *this;
}

SignalDisplay &SignalDisplay::ScrollForward(const GenericSignal &inSignal)
{
    if (mpParent == nullptr)
        return *this;

    if (inSignal.Elements() < 1)
        return *this;

    AdaptTo(inSignal);
    std::unique_lock<std::mutex> dataLock(mDataMutex);
    mShowCursor = false;
    // Shift data to the left, and then append the input signal.
    const auto* pInput = inSignal.ConstData();
    auto* pData = mData.MutableData(GenericSignal::intentRetainContents);
    for (int ch = 0; ch < inSignal.Channels(); ++ch)
    {
        auto* pDataChannel = pData + ch * mData.Elements();
        int count = mData.Elements() - inSignal.Elements();
        if (count > 0) {
            ::memmove(
                pDataChannel,
                pDataChannel + inSignal.Elements(),
                count * sizeof(GenericSignal::ValueType)
            );
        }

        const auto* pInputChannel = pInput + ch * inSignal.Elements();
        if (inSignal.Elements() < mData.Elements()) {
            ::memcpy(
                pDataChannel + mData.Elements() - inSignal.Elements(),
                pInputChannel,
                inSignal.Elements() * sizeof(GenericSignal::ValueType)
            );
        }
        else {
            ::memcpy(
                pDataChannel,
                pInputChannel + inSignal.Elements() - mData.Elements(),
                mData.Elements() * sizeof(GenericSignal::ValueType)
            );
        }
    }
    dataLock.unlock();

    if (mFastScrolling)
    {
        std::unique_lock<std::mutex> bufferLock(mBufferMutex);
        SyncGraphics();
        float delta = SampleLeft(mData.Elements()) - SampleLeft(mData.Elements() - inSignal.Elements());
        delta = Dither(delta);
        {
            QPainter painter(&mImageBuffer);
            dataLock.lock();
            QRectF clipRect(mDataLeft, 0, mDataWidth, mDisplayRect.height());
            dataLock.unlock();
            painter.setClipRect(clipRect);
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawImage(-delta, 0, mImageBuffer);
        }
        bufferLock.unlock();

        QRect redrawRect = mDataRect;
        dataLock.lock();
        redrawRect.setLeft(SampleLeft(mData.Elements() - inSignal.Elements() - 1));
        dataLock.unlock();
        QRegion redrawRegion = redrawRect;
        PaintToBuffer(&redrawRegion);
        InvalidateWidget();
    }
    else
    {
        Invalidate();
    }
    return *this;
}

SignalDisplay &SignalDisplay::ScrollBack(const GenericSignal &inSignal)
{
    if (mpParent == nullptr)
        return *this;

    if (inSignal.Elements() < 1)
        return *this;

    AdaptTo(inSignal);
    std::unique_lock<std::mutex> dataLock(mDataMutex);
    mShowCursor = false;
    // Shift data to the right, and then prepend the input signal.
    const auto* pInput = inSignal.ConstData();
    auto* pData = mData.MutableData(GenericSignal::intentRetainContents);
    for (int ch = 0; ch < inSignal.Channels(); ++ch)
    {
        auto* pDataChannel = pData + ch * mData.Elements();
        int count = mData.Elements() - inSignal.Elements();
        if (count > 0) {
            ::memmove(
                pDataChannel + inSignal.Elements(),
                pDataChannel,
                count * sizeof(GenericSignal::ValueType)
            );
        }

        const auto* pInputChannel = pInput + ch * inSignal.Elements();
        if (inSignal.Elements() < mData.Elements()) {
            ::memcpy(
                pDataChannel,
                pInputChannel,
                inSignal.Elements() * sizeof(GenericSignal::ValueType)
            );
        }
        else {
            ::memcpy(
                pDataChannel,
                pInputChannel,
                mData.Elements() * sizeof(GenericSignal::ValueType)
            );
        }
    }
    dataLock.unlock();

    if (mFastScrolling)
    {
        std::unique_lock<std::mutex> bufferLock(mBufferMutex);
        SyncGraphics();
        float delta = SampleLeft(inSignal.Elements()) - SampleLeft(0);
        delta = Dither(delta);
        {
            QPainter painter(&mImageBuffer);
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawImage(delta, 0, mImageBuffer);
        }
        bufferLock.unlock();

        dataLock.lock();
        QRect redrawRect = mDataRect;
        dataLock.unlock();
        redrawRect.setRight(SampleLeft(inSignal.Elements() + 1));
        QRegion redrawRegion = redrawRect;
        PaintToBuffer(&redrawRegion);
        InvalidateWidget();
    }
    else
    {
        Invalidate();
    }
    return *this;
}

QFont SignalDisplay::AxisFont()
{
    QFont font;
    font.setPixelSize(mFontSize);
    font.setStyleHint(QFont::SansSerif, QFont::PreferBitmap);
    return font;
}

QFont SignalDisplay::MonoFont()
{
    QFont font;
    font.setPixelSize(mFontSize);
    font.setStyleHint(QFont::Monospace, QFont::PreferBitmap);
    return font;
}

QFont SignalDisplay::SymbolFont()
{
    QFont font;
    font.setPixelSize(mFontSize);
    font.setStyleHint(QFont::SansSerif, QFont::PreferBitmap);
    return font;
}

void SignalDisplay::SyncLabelWidth()
{
    mChannelNameCache.clear();
    if (!mpParent)
    {
        mTickLength = cTickLength;
        mLabelWidth = cLabelWidth;
        mMarkerHeight = mLabelWidth;
    }
    else
    {
        QFontMetrics fontMetrics(AxisFont());
        std::ostringstream oss;
        oss << mData.Channels();
        QSize size = fontMetrics.size(0, oss.str().c_str());
        mLabelWidth = 0;
        mMarkerHeight = size.height();
        mTickLength = cTickLength;

        if (mYTicksVisible || mChannelGroupSize > 1)
        {
            mLabelWidth = size.width();
            mMarkerHeight = size.height();
            mTickLength = Round(size.height() / 3.0);
            if (mShowChannelLabels)
            {
                for (size_t i = 0; i < mChannelLabels.size(); ++i)
                {
                    int address = mChannelLabels[i].Address();
                    if (address >= int(mChannelNameCache.size()))
                        mChannelNameCache.resize(address + 1, "");
                    const std::string &label = mChannelLabels[i].Text();
                    mChannelNameCache[address] = label;
                    if (address >= mData.Channels() - mMarkerChannels)
                        mChannelNameCache[address] += " ";
                }
            }
            for (int i = 0; i < mData.Channels(); ++i)
            {
                if (int(mChannelNameCache.size()) <= i)
                    mChannelNameCache.resize(i + 1);
                if (mChannelNameCache[i].empty())
                {
                    std::ostringstream oss;
                    if (mChannelUnit.Symbol().empty())
                        oss << i + cChannelBase;
                    else
                        oss << std::fixed << std::setprecision(1) << mChannelUnit.RawToPhysical(i).Utf8();
                    mChannelNameCache[i] = oss.str();
                }
                const std::string &label = mChannelNameCache[i];
                size = fontMetrics.size(0, label.c_str());
                if (size.width() > mLabelWidth)
                    mLabelWidth = size.width();
            }
        }
        for (const auto& marker : mYAxisMarkers)
        {
            auto def = ParseMarkerDef(marker.Text(), mLightMode);
            size = fontMetrics.size(0, def.label.c_str());
            if (size.width() > mLabelWidth)
                mLabelWidth = size.width();
        }
        if (!mChannelLabelWidth.empty())
            mLabelWidth = fontMetrics.size(0, mChannelLabelWidth.c_str()).width();
        mLabelWidth += 3 * cAxisWidth + cTickWidth;
    }
    if (!mShowNumericValues)
        mNumericValueWidth = 0;
    mDataLeft = mLabelWidth + mNumericValueWidth + cAxisWidth + mTickLength;
    if (mShowNumericValues && mChannelGroupSize == 1 && mYTicksVisible)
        mDataLeft += cAxisWidth;
}

inline void SignalDisplay::SyncGraphics()
{
    mDataRect.setLeft(mDataLeft);
    mDataRect.setTop(0);
    mDataRect.setRight(mDisplayRect.right() - mDisplayRect.left());
    mDataRect.setBottom(mDisplayRect.bottom() - mDisplayRect.top());
    mDataWidth = std::max<int>(0, mDataRect.right() - mDataRect.left());
    mDataHeight = std::max<int>(0, mDataRect.bottom() - mDataRect.top() - mMarkerHeight * mMarkerChannels - mFontSize -
                                       cAxisWidth - 2 * mTickLength);
}

SignalDisplay &SignalDisplay::SetDisplayGroups(int inDisplayGroups)
{
    mMaxDisplayGroups = inDisplayGroups;
    AdjustDisplayGroups();
    return *this;
}

void SignalDisplay::AdjustDisplayGroups()
{
    mNumDisplayGroups = mMaxDisplayGroups;
    if (mMaxDisplayGroups >= mChannelGroups.size())
        mNumDisplayGroups = mChannelGroups.size();
    if (mNumDisplayGroups > mChannelGroups.size())
        mNumDisplayGroups = mChannelGroups.size();
    SetTopGroup(mTopGroup);
    Invalidate();
}

void SignalDisplay::SetSampleLogScale(double s)
{
    if (s != mSampleLogScale) {
        mSampleLogScale = s;
        Invalidate();
    }
}

double SignalDisplay::SampleLogScale() const
{
    return mSampleLogScale;
}

void SignalDisplay::SetValueLogScale(double s)
{
    if (s != mValueLogScale) {
        mValueLogScale = s;
        Invalidate();
    }
}

double SignalDisplay::ValueLogScale() const
{
    return mValueLogScale;
}

SignalDisplay &SignalDisplay::SetTopGroup(int inTopGroup)
{
    int newTopGroup = inTopGroup,
        maxTopGroup = mChannelGroups.size() - mNumDisplayGroups;
    if (newTopGroup > maxTopGroup)
        newTopGroup = maxTopGroup;
    if (newTopGroup < 0)
        newTopGroup = 0;
    if (newTopGroup != mTopGroup)
    {
        mTopGroup = newTopGroup;
        Invalidate();
    }
    return *this;
}

SignalDisplay &SignalDisplay::SetDisplayMode(int mode)
{
    if (mode != mDisplayMode)
    {
        mDisplayMode = eDisplayMode(mode);
        Invalidate();
    }
    return *this;
}

SignalDisplay &SignalDisplay::SetNumSamples(int inNumSamples)
{
    int newNumSamples = inNumSamples;
    if (newNumSamples < 0)
        newNumSamples = 0;
    if (newNumSamples != mNumSamples)
    {
        std::unique_lock<std::mutex> lock(mDataMutex);
        GenericSignal newData(mData.Channels(), newNumSamples);
        int idx1 = mCursorPosition, idx2 = newNumSamples - mNumSamples;
        if (idx2 < 0)
        {
            idx1 = (idx1 - idx2) % mNumSamples;
            idx2 = 0;
        }
        for (int i = 0; i < idx2; ++i)
            for (int ch = 0; ch < newData.Channels(); ++ch)
                newData(ch, i) = NaN(newData(ch, i));
        while (idx2 < newNumSamples)
        {
            for (int ch = 0; ch < newData.Channels(); ++ch)
                newData(ch, idx2) = mData(ch, idx1);
            ++idx1 %= mNumSamples;
            ++idx2;
        }
        mData = newData;
        mCursorPosition = 0;
        mNumSamples = newNumSamples;
        lock.unlock();
        Invalidate();
    }
    return *this;
}

void SignalDisplay::PaintToBuffer(const void* inUpdateRgn)
{
    const QRegion* pUpdateRgn = static_cast<const QRegion*>(inUpdateRgn);
    if (pUpdateRgn && pUpdateRgn->isEmpty())
        return;

    std::unique_lock<std::mutex> dataLock(mDataMutex);
    std::unique_lock<std::mutex> bufferLock(mBufferMutex);
    PaintData p;
    SetupPainting(p, &mImageBuffer, inUpdateRgn);
    QPainter::CompositionMode mode = p.painter->compositionMode();
    p.painter->setCompositionMode(QPainter::CompositionMode_Source);
    p.painter->fillRect(mDisplayRect, Qt::transparent);
    p.painter->setCompositionMode(mode);

    std::vector<QRegion> rgns;
    if (inUpdateRgn)
        for (const auto& rect : *pUpdateRgn)
            rgns.push_back(rect);
    else
        rgns.push_back(mDataRect);

    for (const auto& rgn : rgns)
    {
        p.updateRgn = &rgn;
        p.painter->setClipRegion(rgn);
        switch (mDisplayMode)
        {
        case polyline:
            DrawSignalPolyline(p);
            break;
        case field2d:
            DrawSignalField2d(p);
            break;
        default:
            Assert(false);
        }
    }
    CleanupPainting(p);
}

SignalDisplay &SignalDisplay::Paint(const void *inUpdateRgn)
{
    std::unique_lock<std::mutex> lock(mBufferMutex);
    QRegion rgn = mInvalidBufferRgn;
    mInvalidBufferRgn = QRegion();
    lock.unlock();

    PaintToBuffer(&rgn);

    if (mpParent == nullptr)
        return *this;

    PaintData p;
    SetupPainting(p, mpParent, inUpdateRgn);
    ClearBackground(p);
    if (mDisplayMode == polyline) {
        DrawBaselinesAndGrid(p);
        DrawSelection(p);
    }
    lock.lock();
    p.painter->drawImage(0, 0, mImageBuffer);
    lock.unlock();
    if (mXTicksVisible)
        DrawXTicks(p);
    DrawAxes(p);
    switch (mDisplayMode)
    {
    case polyline:
        DrawYLabels(p, true);
        break;
    case field2d:
        DrawYLabels(p, false);
        break;
    default:
        Assert(false);
    }

#ifdef TEST_UPDATE_RGN
    p.painter->fillRect(mDisplayRect, QColor(::rand() << 8 & 0x00ffff00));
#endif // TEST_UPDATE_RGN

    DrawMarkerChannels(p);
    DrawChannelLabels(p);
    DrawCursor(p);
    DrawMarkers(p);
    if (mDisplayMode == polyline) {
        DrawValueUnits(p);
    }
    CleanupPainting(p);
    return *this;
}

void SignalDisplay::RebuildChannelGroups()
{
    mChannelGroups.clear();

    std::vector<int> group;
    for (int channel = 0; channel < mChannelVisible.size(); ++channel)
    {
        if (mChannelVisible[channel])
            group.push_back(channel);

        if (channel % mChannelGroupSize == mChannelGroupSize - 1 && !group.empty())
        {
            mChannelGroups.push_back(group);
            group.clear();
        }
    }
    if (!group.empty())
        mChannelGroups.push_back(group);

    // Test if channel groups are trivial, i.e. all of the same size
    // and in order
    if (mChannelGroups.empty()) {
        mTrivialChannelGroups = false;
        return;
    }
    size_t channelGroupSize = mChannelGroups.front().size();
    for (int i = 1; i < mChannelGroups.size(); ++i) {
        if (mChannelGroups[i].size() != channelGroupSize) {
            mTrivialChannelGroups = false;
            return;
        }
    }
    int currentChannel = 0;
    for (const auto& group : mChannelGroups) {
        for (const auto& channel : group) {
            if (channel != currentChannel++) {
                mTrivialChannelGroups = false;
                return;
            }
        }
    }
    mTrivialChannelGroups = true;
}

float SignalDisplay::ChannelTop(int ch) const
{
    return GroupTop(ChannelToGroup(ch) - mTopGroup);
}

float SignalDisplay::ChannelBottom(int ch) const
{
    return GroupBottom(ChannelToGroup(ch) - mTopGroup);
}

int SignalDisplay::PosToChannel(float p) const
{
    return GroupToChannel(PosToGroup(p) + mTopGroup);
}

int SignalDisplay::PosToGroup(float p) const
{
    if (p < GroupTop(0))
        return -1;
    if (p >= GroupBottom(mNumDisplayGroups - 1))
        return -1;
    return (p - GroupTop(0)) / (GroupTop(1) - GroupTop(0));
}

int SignalDisplay::ChannelToGroup(int inCh) const
{
    if (mTrivialChannelGroups) {
        return inCh / mChannelGroupSize;
    }
    for (int group = 0; group < mChannelGroups.size(); ++group)
        for (int channel : mChannelGroups[group])
            if (channel == inCh)
                return group;
    return -1;
}

int SignalDisplay::GroupToChannel(int g) const
{
    return g < mChannelGroups.size() ? mChannelGroups[g].front() : -1;
}

double SignalDisplay::NormData(size_t ch, size_t el) const
{
    double minValue = mMinValues.size() > ch ? mMinValues[ch] : mMinValue;
    double maxValue = mMaxValues.size() > ch ? mMaxValues[ch] : mMaxValue;
    double normValue = (mData(ch, el) - minValue) / (maxValue - minValue);
    if (mValueLogScale > 0)
        return ForwardLogTransform(normValue, 1, mValueLogScale);
    return normValue;
}

std::string SignalDisplay::GetNumericValue(int ch) const
{
    if (ch >= mData.Channels() || mData.Elements() < 1)
        return " ";
    int sampleNumber = mCursorPosition - 1;
    if (sampleNumber < 0)
        sampleNumber += mData.Elements();
    double value = mData(ch, sampleNumber);

    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    bool channelShowValueUnit = mValueUnitsVisible.size() > ch ? mValueUnitsVisible[ch] : false;
    if (mShowValueUnit || channelShowValueUnit)
    {
        const PhysicalUnit& unit = mValueUnits.size() > ch ? mValueUnits[ch] : mValueUnit;
        oss.precision(2);
        oss.width(6);
        if (value == Inf(value) || value == -Inf(value) || value == 0 || IsNaN(value))
            oss << Pretty(value);
        else
            oss << unit.RawToPhysical(value).Utf8();
    }
    else
    {
        oss.precision(10);
        oss << Pretty(value);
    }
    return oss.str();
}

void SignalDisplay::SetupPainting(PaintData &p, QPaintDevice* pDevice, const void *inUpdateRgn)
{
    SyncGraphics();

    p.updateRgn = static_cast<const QRegion *>(inUpdateRgn);
    p.painter = new QPainter(pDevice);
    if (p.updateRgn)
        p.painter->setClipRegion(*p.updateRgn);
    p.painter->setRenderHint(QPainter::Antialiasing, false);
    p.painter->setRenderHint(QPainter::TextAntialiasing, false);
    p.signalPens.resize(mData.Channels());
    p.signalBrushes.resize(mData.Channels());
    p.signalStyles.resize(mData.Channels());

    // Background properties
    RGBColor backgroundColor = mLightMode ? RGBColor::LtYellow : RGBColor::Black;
    p.backgroundColor = QColor(backgroundColor.R(), backgroundColor.G(), backgroundColor.B());
    p.backgroundBrush = QBrush(p.backgroundColor);

    // Cursor properties
    RGBColor c = mLightMode ? RGBColor::DkGray : mCursorColor;
    p.cursorColor = QColor(c.R(), c.G(), c.B());
    p.cursorBrush = QBrush(p.cursorColor);

    // Axis properties
    RGBColor axisColor = mAxisColor;
    if (mLightMode && (axisColor == cAxisColorDefault))
        axisColor = RGBColor::Black;
    else if (mLightMode && axisColor.IsGray())
        axisColor = axisColor.InvertLuminance();
    QColor qAxisColor(axisColor.R(), axisColor.G(), axisColor.B());
    p.axisBrush = QBrush(qAxisColor);
    p.axisY = GroupBottom(std::max(0, mNumDisplayGroups - 1));

    RGBColor gridColor = mLightMode ? RGBColor::LtGray : RGBColor::DkGray;
    QColor qGridColor(gridColor.R(), gridColor.G(), gridColor.B());

    p.markerWidth = 1;
    p.baselinePen = QPen(qAxisColor);
    p.gridPen = QPen(qGridColor, 1, Qt::DashLine);
    p.gridPen.setDashPattern({ 17, 4 }); // use large pattern to avoid cost of too many dashes
    c = RGBColor(mLightMode ? RGBColor::Black : RGBColor::White);
    p.markerColor = QColor(c.R(), c.G(), c.B());
    p.markerBrush = QBrush(p.markerColor);
    p.labelColor = qAxisColor;
    p.labelFont = AxisFont();
    p.monoFont = MonoFont();
    p.symbolFont = SymbolFont();
    mSymbolWidth = QFontMetrics(p.symbolFont).maxWidth();

    // Signal properties
    if (mDisplayColors)
        for (int i = 0; i < mData.Channels(); ++i)
        {
            RGBColor channelColor = ChannelColor(i);
            channelColor = RGBColor::EnsureColorContrast(channelColor, backgroundColor);
            QColor c(channelColor.R(), channelColor.G(), channelColor.B());
            p.signalPens[i].setColor(c);
            p.signalBrushes[i].setColor(c);
        }
    else
    {
        RGBColor channelColor = mLightMode ? RGBColor::Black : RGBColor::White;
        QColor c(channelColor.R(), channelColor.G(), channelColor.B());
        QPen pen(c);
        for (int i = 0; i < mData.Channels(); ++i)
            p.signalPens[i] = pen;
    }
    p.signalStyles = QString::fromUtf8(mChannelStyles.c_str());
    while (p.signalStyles.length() < mData.Channels())
        p.signalStyles.append('-');
}

void SignalDisplay::ClearBackground(const PaintData &p)
{
    p.painter->fillRect(mDisplayRect, p.backgroundColor);
}

static void DrawLine(QPainter *p, const QString &style, const QPointF *points, int pointCount, int decimation = 1)
{
    if (style == "-")
    {
        int i = 0;
        for (i = 0; i < pointCount - decimation; i += decimation)
        {
          qreal x1 = points[i].x(), x2 = points[i + decimation].x();
          if (decimation > 1)
          {
            qreal ymin = Inf<float>(), ymax = -Inf<float>();
            for (int j = 0; j < decimation; ++j)
            {
              ymin = std::min(points[i + j].y(), ymin);
              ymax = std::max(points[i + j].y(), ymax);
            }
            p->drawLine((x1 + x2) / 2, ymin, (x1 + x2) / 2, ymax);
          }
          p->drawLine(points[i], points[i + decimation]);
        }
        if (i < pointCount - 1)
        {
          qreal x1 = points[i].x(), x2 = points[pointCount - 1].x();
          if (pointCount - i > 1)
          {
            qreal ymin = Inf<float>(), ymax = -Inf<float>();
            for (int j = 0; j < pointCount - i; ++j)
            {
              ymin = std::min(points[i + j].y(), ymin);
              ymax = std::max(points[i + j].y(), ymax);
            }
            p->drawLine((x1 + x2) / 2, ymin, (x1 + x2) / 2, ymax);
          }
          p->drawLine(points[i], points[pointCount - 1]);
        }
    }
    else
    {
        QPoint delta = p->fontMetrics().tightBoundingRect(style).center();
        for (int i = 0; i < pointCount; i += decimation)
            p->drawText(points[i] - delta, style);
    }
}
void SignalDisplay::DrawBaselinesAndGrid(const PaintData& p)
{
    // Baselines and horizontal grid
    float baseInterval = mNumDisplayGroups > 0 ? mDataHeight / mNumDisplayGroups : mDataHeight;
    std::vector<float> primaryLines;
    primaryLines.reserve(mNumDisplayGroups);
    for (int i = 0; i < mNumDisplayGroups; ++i)
    {
        int ch = GroupToChannel(i);
        double minValue = mMinValues.size() > ch ? mMinValues[ch] : mMinValue;
        double maxValue = mMaxValues.size() > ch ? mMaxValues[ch] : mMaxValue;
        float offset = minValue / (maxValue - minValue);
        primaryLines.push_back(GroupBottom(i) + baseInterval * offset);
    }

    QPointF points[2];
    points[0].setX(SampleLeft(0));
    points[1].setX(SampleRight(mNumSamples));

    if (mShowBaselines || mShowGrid) {
        p.painter->setPen(mShowBaselines ? p.baselinePen : p.gridPen);
        for (float yLine : primaryLines) {
            points[0].setY(yLine);
            points[1].setY(yLine);
            p.painter->drawPolyline(points, 2);
        }
    }

    if (mShowGrid) {
        std::vector<float> secondaryLines = ComputeSecondaryGridLines(
            primaryLines, GroupTop(0), GroupBottom(mNumDisplayGroups - 1), cMinGridDist);
        p.painter->setPen(p.gridPen);
        for (float yLine : secondaryLines) {
            points[0].setY(yLine);
            points[1].setY(yLine);
            p.painter->drawPolyline(points, 2);
        }
    }

    primaryLines.clear();
    auto div = GetXDivision();
    float prevLine = -Inf<float>();
    for (float sample = div.first; sample < float(mNumSamples); sample += div.second) {
        float curLine = SampleLeft(ToInt(sample));
        if (curLine - prevLine >= cMinGridDist) {
            primaryLines.push_back(curLine);
            prevLine = curLine;
        }
    }
    points[0].setY(GroupBottom(mNumDisplayGroups - 1));
    points[1].setY(GroupTop(0));
    if (mShowGrid) {
        p.painter->setPen(p.gridPen);
        for (float xLine : primaryLines) {
            points[0].setX(xLine);
            points[1].setX(xLine);
            p.painter->drawPolyline(points, 2);
        }
        float left = SampleLeft(0), right = SampleLeft(mNumSamples - 1);
        if (primaryLines.size() > 1) {
            float delta = (primaryLines[1] - primaryLines[0]);
            left = primaryLines.front() - delta;
            right = primaryLines.back() + delta;
        }
        std::vector<float> secondaryLines = ComputeSecondaryGridLines(primaryLines, left, right, cMinGridDist);
        for (float xLine : secondaryLines) {
            if (xLine > mDataLeft) {
                points[0].setX(xLine);
                points[1].setX(xLine);
                p.painter->drawPolyline(points, 2);
            }
        }
    }

}

void SignalDisplay::DrawSignalPolyline(const PaintData &p)
{
    float baseInterval = mNumDisplayGroups > 0 ? mDataHeight / mNumDisplayGroups : mDataHeight;
    double maxY = mDataRect.bottom() + 1, minY = mDataRect.top() - 1;

    int sampleBegin = 0, sampleEnd = mNumSamples;
    if (p.updateRgn)
    { // We restrict drawing to the actually requested update region.
        QRect clipRect = p.updateRgn->boundingRect();
        sampleBegin = ::floor(PosToSample(clipRect.left()) - 2);
        sampleBegin = std::max(sampleBegin - 1, 0);
        sampleEnd = ::ceil(PosToSample(clipRect.right()) + 2);
        sampleEnd = std::min(sampleEnd + 1, mNumSamples);
    }
    if (mNumSamples != mSignalPointsCount)
    {
        delete[] mpSignalPoints;
        try
        {
            mpSignalPoints = new QPointF[mNumSamples];
            mSignalPointsCount = mNumSamples;
        }
        catch (const std::bad_alloc&)
        {
            mpSignalPoints = nullptr;
            mSignalPointsCount = 0;
            throw std_bad_alloc << "Could not allocate memory for " << mNumSamples << " points";
        }
    }
    p.painter->setFont(p.symbolFont);
    int numPens = static_cast<int>(p.signalPens.size());

    int decimation = 1;
    if (mAutoDecimationMaxLinesPerPixel > 0 && mDataRect.width() > 0)
    {
        int linesPerPixel = mNumSamples / mDataRect.width();
        if (linesPerPixel > mAutoDecimationMaxLinesPerPixel)
            decimation = linesPerPixel / mAutoDecimationMaxLinesPerPixel;
    }

    for (int j = sampleBegin; j < sampleEnd; ++j)
        mpSignalPoints[j].setX(SampleLeft(j));

    for (int group = mTopGroup; group < mTopGroup + mNumDisplayGroups; ++group)
    {
        for (int channel : mChannelGroups[group])
        {
            int channelBottom = ChannelBottom(channel);
            QString style = p.signalStyles[channel % p.signalStyles.length()];
            p.painter->setPen(p.signalPens[channel % numPens]);
            int runEnd = sampleBegin;
            while (runEnd < sampleEnd)
            {
                int runBegin = runEnd;
                while (runBegin < sampleEnd && IsNaN(NormData(channel, runBegin)))
                    ++runBegin;
                double value;
                runEnd = runBegin;
                while (runEnd < sampleEnd && !IsNaN(value = NormData(channel, runEnd)))
                {
                    double y = channelBottom - 1 - baseInterval * value;
                    if (y > maxY)
                        y = maxY;
                    else if (y < minY)
                        y = minY;
                    mpSignalPoints[runEnd++].setY(y);
                }
                if (runBegin <= mCursorPosition && mCursorPosition < runEnd)
                {
                    DrawLine(p.painter, style, mpSignalPoints + runBegin, mCursorPosition - runBegin, decimation);
                    DrawLine(p.painter, style, mpSignalPoints + mCursorPosition, runEnd - mCursorPosition, decimation);
                }
                else
                {
                    DrawLine(p.painter, style, mpSignalPoints + runBegin, runEnd - runBegin, decimation);
                }
            }
        }
    }
}

void SignalDisplay::DrawSignalField2d(const PaintData &p)
{
    int sampleBegin = 0, sampleEnd = mNumSamples;
    if (p.updateRgn)
    { // We restrict drawing to the actually requested update region.
        QRect clipRect = p.updateRgn->boundingRect();
        sampleBegin = PosToSample(clipRect.left());
        sampleBegin = std::max(sampleBegin - 1, 0);
        sampleEnd = PosToSample(clipRect.right() + 1);
        sampleEnd = std::min(sampleEnd + 1, mNumSamples);
    }
    for (int group = mTopGroup; group < mTopGroup + mNumDisplayGroups; ++group)
    {
        for (int channel : mChannelGroups[group])
        {
            for (int sample = sampleBegin; sample < sampleEnd; ++sample)
            {
                bool draw = true;
                double dataValue = NormData(channel, sample);
                if (dataValue < 0.0)
                    dataValue = 0.0;
                else if (dataValue > 1.0)
                    dataValue = 1.0;

                QRectF dotRect(SampleLeft(sample), ChannelTop(channel), 
                               SampleRight(sample) - SampleLeft(sample),
                               ChannelBottom(channel) - ChannelTop(channel)
                               );
                RGBColor rgb;
                if (IsNaN(dataValue))
                    rgb = RGBColor::Black;
                else if (!mValueColors.empty())
                {
                    typedef decltype(*mValueColors.begin()) pair;
                    auto i = std::lower_bound(mValueColors.begin(), mValueColors.end(), dataValue,
                                              [](pair &a, double b) { return a.first < b; });
                    if (i == mValueColors.begin())
                        rgb = i->second;
                    else if (i == mValueColors.end())
                        rgb = mValueColors.back().second;
                    else
                    {
                        auto j = i--;
                        double p = dataValue / (j->first - i->first);
                        rgb = (1 - p) * i->second + p * j->second;
                    }
                }
                else if (mDisplayColors)
                    rgb = RGBColor::FromHSV(dataValue - 1.0 / 3.0, 1.0, dataValue);
                else
                    rgb = RGBColor::FromHSV(0.0, 0.0, dataValue);
                p.painter->fillRect(dotRect, QColor(rgb.R(), rgb.G(), rgb.B()));
            }
        }
    }
}

void SignalDisplay::DrawMarkerChannels(const PaintData& p)
{
    for (int markerCh = 0; markerCh < mMarkerChannels; ++markerCh)
    {
        auto style = mMarkerChannelStyles.size() > markerCh ? mMarkerChannelStyles[markerCh] : byvalue;
        switch (style) {
        case byvalue:
            DrawMarkerChannelByValue(markerCh, p);
            break;
        case bychange:
            DrawMarkerChannelByChange(markerCh, p);
            break;
        default:
            ;
        }
    }
}

void SignalDisplay::DrawMarkerChannelByChange(int markerCh, const PaintData& p)
{
    p.painter->setPen(p.labelColor);
    p.painter->setFont(p.labelFont);
    p.painter->setBackgroundMode(Qt::TransparentMode);
    int baseline = MarkerChannelBottom(markerCh);
    size_t channelNumber = mData.Channels() - mMarkerChannels + markerCh;
    if (mData.Elements() > 0)
    {
        uint32_t prevVal = ToInt(mData(channelNumber, 0));
        for (int sample = 0; sample < mData.Elements(); ++sample)
        {
            uint32_t curVal = ToInt(mData(channelNumber, sample));
            if (curVal != prevVal)
            {
                QRect posRect;
                posRect.setLeft(SampleLeft(sample) - p.markerWidth);
                posRect.setTop(baseline);
                posRect.setRight(SampleLeft(sample));
                posRect.setBottom(baseline);
                std::ostringstream oss;
                oss << curVal;
                p.painter->drawText(posRect,
                                    Qt::AlignHCenter | Qt::TextSingleLine | Qt::AlignBottom | Qt::TextDontClip,
                                    oss.str().c_str());
                QRect markerRect;
                markerRect.setLeft(SampleLeft(sample) - p.markerWidth);
                markerRect.setTop(GroupTop(0));
                markerRect.setRight(SampleLeft(sample));
                markerRect.setBottom(p.axisY);
                p.painter->fillRect(markerRect, p.axisBrush);
                prevVal = curVal;
            }
        }
    }
    p.painter->setBackground(p.backgroundBrush);
    p.painter->setBackgroundMode(Qt::OpaqueMode);
    QRect labelRect(cTickWidth, baseline, 0, 0);
    const char *labelText = "";
    if (mChannelNameCache.size() > channelNumber)
        labelText = mChannelNameCache[channelNumber].c_str();
    p.painter->drawText(labelRect, Qt::AlignLeft | Qt::TextSingleLine | Qt::AlignBottom | Qt::TextDontClip,
                        labelText);
}

void SignalDisplay::DrawMarkerChannelByValue(int markerCh, const PaintData& p)
{
    p.painter->save();
    p.painter->setRenderHint(QPainter::Antialiasing);
    p.painter->setPen(QPen(p.labelColor, 1.5));
    p.painter->setFont(p.labelFont);
    p.painter->setBackgroundMode(Qt::TransparentMode);
    auto bottom = MarkerChannelBottom(markerCh), top = MarkerChannelTop(markerCh);
    size_t channelNumber = mData.Channels() - mMarkerChannels + markerCh;
    if (mData.Elements() > 0)
    {
        uint32_t maxval = 0, minval = std::numeric_limits<uint32_t>::max();
        for (int i = 0; i < mData.Elements(); ++i) {
            maxval = std::max<uint32_t>(maxval, ToInt(mData(channelNumber, i)));
            minval = std::min<uint32_t>(minval, ToInt(mData(channelNumber, i)));
        }
        float ratio = top == bottom ? 0 : (top - bottom - 1) / (maxval - minval + 1);
        QPointF* polyline = new QPointF[mData.Elements()];
        for (int i = 0; i < mData.Elements(); ++i) {
            auto value = (mData(channelNumber, i) - minval) * ratio + bottom;
            polyline[i].setX(SampleLeft(i));
            polyline[i].setY(value);
        }
        p.painter->drawPolyline(polyline, mData.Elements());
        delete[] polyline;
    }
    p.painter->setBackground(p.backgroundBrush);
    p.painter->setBackgroundMode(Qt::OpaqueMode);
    QRect labelRect(cTickWidth, bottom, 0, 0);
    const char* labelText = "";
    if (mChannelNameCache.size() > channelNumber)
        labelText = mChannelNameCache[channelNumber].c_str();
    p.painter->drawText(labelRect, Qt::AlignLeft | Qt::TextSingleLine | Qt::AlignBottom | Qt::TextDontClip,
        labelText);
    p.painter->restore();
}

QRect SignalDisplay::CursorRect(int currentSamplePosition) const
{
    // The cursor must be shown to the left of the current sample position
    int cursorPosition = currentSamplePosition - 1;
    if (cursorPosition < 0)
        cursorPosition += mData.Elements();
    int cursorLeft = SampleLeft(cursorPosition);
    return QRect(cursorLeft, 0, mCursorWidth, GroupBottom(std::max(0, mNumDisplayGroups - 1)));
}

std::vector<float> SignalDisplay::ComputeSecondaryGridLines(const std::vector<float>& inPrimaryLines, float inMin, float inMax, int inMinGridDist) const
{
    std::vector<float> allGridLines;
    allGridLines.reserve(inPrimaryLines.size() + 2);
    allGridLines.push_back(inMin);
    allGridLines.insert(allGridLines.end(), inPrimaryLines.begin(), inPrimaryLines.end());
    allGridLines.push_back(inMax);
    // Assume that inPrimaryLines is sorted.
    std::vector<float> newGridLines, secondaryGridLines;
    do {
        newGridLines.clear();
        for (int i = 0; i < allGridLines.size() - 1; ++i) {
            float v1 = allGridLines[i], v2 = allGridLines[i + 1];
            if ((v2 - v1) / 2 >= inMinGridDist)
                newGridLines.push_back((v1 + v2) / 2);
        }
        allGridLines = MergeSortedLists(allGridLines, newGridLines);
        secondaryGridLines = MergeSortedLists(secondaryGridLines, newGridLines);
    } while (!newGridLines.empty());
    return secondaryGridLines;
}

std::pair<float, float> SignalDisplay::GetXDivision() const
{
    float xStart = 0, xDivision = 1;
    if (mNumSamples > 0)
    { // Are samples spaced too dense to allow for individual labels?
        float pixelsPerSample = (SampleLeft(mNumSamples - 1) - SampleLeft(0)) / mNumSamples;
        if (pixelsPerSample < 30)
        { // Samples are dense
            float displayLength = ::fabs(mNumSamples * mSampleUnit.Gain()),
                  scale = ::pow(10.0, ::floor(::log10(displayLength) + 0.5));
            xDivision = scale / mSampleUnit.Gain() / 5;
            xStart = xDivision - ::fmod(float(-mSampleUnit.Offset()), xDivision);
        }
    }
    if (xDivision < 1)
        xDivision = 1;

    return std::make_pair(xStart, xDivision);
}

void SignalDisplay::DrawCursor(const PaintData &p)
{
    if (mShowCursor)
    {
        QRect cursorRect = CursorRect(mCursorPosition);
        p.painter->fillRect(cursorRect, p.cursorBrush);
        // Remember the cursor rectangle for redrawing when the next
        // data packet arrives.
        mPrevCursorRect = cursorRect;
    }
}

void SignalDisplay::DrawXTicks(const PaintData &p)
{
    p.painter->setPen(p.labelColor);
    p.painter->setFont(p.labelFont);
    p.painter->setBackgroundMode(Qt::TransparentMode);

    auto div = GetXDivision();
    float xStart = div.first, xDivision = div.second;
 
    int nextLabelPos = mDataRect.left();
    for (float j = xStart; j < float(mNumSamples); j += xDivision)
    {
        float tickX = 0;
        switch (mDisplayMode)
        {
        case field2d:
            tickX = (SampleRight(ToInt(j)) + SampleLeft(ToInt(j))) / 2;
            break;
        case polyline:
        default:
            tickX = SampleLeft(ToInt(j));
        }
        QRectF tickRect(tickX - cTickWidth / 2, p.axisY + cAxisWidth, cTickWidth, mTickLength);
        p.painter->fillRect(tickRect, p.axisBrush);
        if (tickX > nextLabelPos)
        {
            tickRect.setTop(tickRect.top() + (mTickLength + 3) / 2);
            std::ostringstream label;
            if (mTimeLabels)
            {
                float val = (j - mSampleUnit.Offset()) * mSampleUnit.Gain();
                label << ' ' << std::setfill('0') << std::setw(2) << int(val) / 60 << ':' << std::setw(2)
                      << std::setprecision(2) << int(val) % 60;
                if (::fmod(xDivision * mSampleUnit.Gain(), 1.0) != 0.0)
                    label << '.' << std::setw(2) << int(100 * val + 0.5) % 100;
                label << ' ';
            }
            else
            {
                label << ' ' << std::setprecision(6) << mSampleUnit.RawToPhysical(j, mNumSamples).Utf8() << ' ';
            }

            p.painter->drawText(tickRect, Qt::AlignHCenter | Qt::TextSingleLine | Qt::AlignTop | Qt::TextDontClip,
                                label.str().c_str());
            nextLabelPos = tickX + p.painter->fontMetrics().boundingRect(label.str().c_str()).width();
        }
    }
}

void SignalDisplay::DrawYLabels(const PaintData &p, bool inDrawTicks)
{
    p.painter->setPen(p.labelColor);
    p.painter->setBackgroundMode(Qt::TransparentMode);
    int nextLabelPos = mDataRect.top();
    for (int i = 0; i < mNumDisplayGroups; ++i)
    {
        int group = i + mTopGroup;
        size_t channelNumber = GroupToChannel(group);
        int tickY = (GroupTop(i) + GroupBottom(i)) / 2;
        if (inDrawTicks && mYTicksVisible)
        {
            QRect tickRect(mDataLeft - cAxisWidth - mTickLength, tickY - cTickWidth / 2, mTickLength, cTickWidth);
            if (mDisplayColors && mChannelGroupSize == 1)
            {
                tickRect.setTop(tickRect.top() - 1);
                tickRect.setBottom(tickRect.bottom() + 1);
                p.painter->fillRect(tickRect, p.signalBrushes[channelNumber]);
            }
            else
            {
                p.painter->fillRect(tickRect, p.axisBrush);
            }
        }
        if (tickY >= nextLabelPos)
        {
            const char *labelText = "";
            if (mChannelGroupSize == 1 && mChannelNameCache.size() > channelNumber)
                labelText = mChannelNameCache[channelNumber].c_str();
            if (mChannelGroupSize == 1)
            {
                QRect r(0, tickY - cTickWidth / 2, 0, 0);
                if (mShowNumericValues)
                    r.setRight(NumericValuesLeft() - 2 * cAxisWidth);
                else
                    r.setRight(NumericValuesLeft());
                p.painter->setFont(p.labelFont);
                if (mYTicksVisible)
                {
                    p.painter->setPen(p.labelColor);
                    p.painter->drawText(r, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight | Qt::TextDontClip,
                      labelText);
                }
                if (mShowNumericValues)
                {
                    p.painter->setPen(p.signalPens[channelNumber]);
                    r.setRight(NumericValuesRight());
                    DrawNumericValue(p, r, GetNumericValue(channelNumber).c_str());
                }
            }
            nextLabelPos = tickY + p.painter->fontMetrics().height();
        }
    }
}

void SignalDisplay::DrawAxes(const PaintData &p)
{
    QRect xAxis(0, p.axisY, mDisplayRect.right(), cAxisWidth);
    p.painter->fillRect(xAxis, p.axisBrush);
    QRect yAxis;
    yAxis.setLeft(mDataLeft - cAxisWidth);
    yAxis.setTop(mMarkerChannels > 0 ? GroupTop(0) : 0);
    yAxis.setWidth(cAxisWidth);
    yAxis.setBottom(mDisplayRect.bottom());
    p.painter->fillRect(yAxis, p.axisBrush);
    if (mChannelGroupSize == 1 && mShowNumericValues)
    {
        QRect divider(NumericValuesLeft() - cAxisWidth, yAxis.top(), cAxisWidth, yAxis.height());
        p.painter->fillRect(divider, p.axisBrush);
    }
    if (mMarkerChannels > 0)
    {
        QRect divider;
        divider.setLeft(0);
        divider.setRight(mDisplayRect.right());
        divider.setTop(GroupTop(0) - cAxisWidth);
        divider.setBottom(GroupTop(0));
        p.painter->fillRect(divider, p.axisBrush);
    }
}

void SignalDisplay::DrawMarkers(const PaintData &p)
{
    // Draw Y axis markers.
    const double baseInterval = mNumDisplayGroups > 0 ? mDataHeight / mNumDisplayGroups : mDataHeight;
    for (size_t i = 0; i < mYAxisMarkers.size(); ++i)
    {
        auto def = ParseMarkerDef(mYAxisMarkers[i].Text(), mLightMode);

        QColor markerColor = def.color;
        if (!markerColor.isValid())
            markerColor = p.markerColor;

        const double value = (mYAxisMarkers[i].Address() - mMinValue) / (mMaxValue - mMinValue);
        const double markerY = GroupBottom(DisplayGroups() - 1) - baseInterval * value;

        QRect markerBar;
        markerBar.setLeft(mDataLeft - mTickLength - cAxisWidth);
        markerBar.setTop(markerY - cAxisWidth);
        markerBar.setWidth(mTickLength + cAxisWidth);
        markerBar.setHeight(2 * cAxisWidth);
        p.painter->fillRect(markerBar, markerColor);

        if (!def.label.empty())
        {
            QString qlabel = QString::fromUtf8(def.label.c_str());
            QRect labelRect;
            labelRect.setLeft(markerBar.left());
            labelRect.setRight(labelRect.left());
            labelRect.setTop(markerY - 1);
            labelRect.setBottom(labelRect.top());
            p.painter->save();
            p.painter->setBackground(p.backgroundBrush);
            p.painter->setBackgroundMode(Qt::OpaqueMode);
            p.painter->setPen(markerColor);
            p.painter->drawText(labelRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight | Qt::TextDontClip, qlabel);
            p.painter->restore();
        }

        if (def.style != Qt::NoPen)
        {
            QPen pen;
            pen.setBrush(markerColor);
            pen.setWidthF(markerBar.height() * def.width);
            pen.setStyle(def.style);
            p.painter->save();
            p.painter->setPen(pen);
            p.painter->setBackground(p.backgroundBrush);
            p.painter->setBackgroundMode(Qt::OpaqueMode);
            p.painter->drawLine(mDataLeft, markerY, SampleRight(mNumSamples - 1), markerY);
            p.painter->restore();
        }
    }

    // Draw X axis markers.
    for (size_t i = 0; i < mXAxisMarkers.size(); ++i)
    {
        auto def = ParseMarkerDef(mXAxisMarkers[i].Text(), mLightMode);

        QColor markerColor = def.color;
        if (!markerColor.isValid())
            markerColor = p.markerColor;

        int markerX = SampleRight(ToInt(mXAxisMarkers[i].Address() + mSampleUnit.Offset()));
        QRect markerBar(markerX - cAxisWidth, p.axisY, 2 * cAxisWidth, mTickLength + cAxisWidth);
        p.painter->fillRect(markerBar, markerColor);

        if (!def.label.empty())
        {
            QString qlabel = QString::fromUtf8(def.label.c_str());
            QRect labelRect(markerX, p.axisY + cAxisWidth, 0, cAxisWidth);
            labelRect.setTop(labelRect.top() + (mTickLength + 3) / 2);
            p.painter->save();
            p.painter->setBackground(p.backgroundBrush);
            p.painter->setBackgroundMode(Qt::OpaqueMode);
            p.painter->setPen(markerColor);
            p.painter->drawText(labelRect, Qt::AlignHCenter | Qt::TextSingleLine | Qt::AlignTop | Qt::TextDontClip, qlabel);
            p.painter->restore();
        }

        if (def.style != Qt::NoPen)
        {
            QPen pen;
            pen.setBrush(markerColor);
            pen.setWidthF(markerBar.width() * def.width);
            pen.setStyle(def.style);
            p.painter->save();
            p.painter->setPen(pen);
            p.painter->setBackground(p.backgroundBrush);
            p.painter->setBackgroundMode(Qt::OpaqueMode);
            p.painter->drawLine(markerX, p.axisY, markerX, 0);
            p.painter->restore();
        }
    }
}

void SignalDisplay::DrawChannelLabels(const PaintData &p)
{
    if (mChannelGroupSize > 1)
    { // Draw channel labels when channels don't coincide with groups.
        p.painter->setBackground(p.backgroundColor);
        p.painter->setBackgroundMode(Qt::OpaqueMode);
        int deltaY = QFontMetrics(p.labelFont).height();
        QRect legendRect(0, cAxisWidth, NumericValuesRight(), deltaY);
        for (size_t i = 0; i < mChannelLabels.size(); ++i)
        {
            if (!ChannelVisible(i))
                continue;

            p.painter->fillRect(legendRect, p.backgroundColor);
            RGBColor textColor = ChannelColor(mChannelLabels[i].Address());
            if (mLightMode && textColor.IsGray())
                textColor = textColor.InvertLuminance();
            p.painter->setPen(QColor(textColor.R(), textColor.G(), textColor.B()));
            if (mShowChannelLabels)
            {
                p.painter->setFont(p.labelFont);
                p.painter->drawText(legendRect.adjusted(cAxisWidth, 0, 0, 0),
                                    Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignLeft | Qt::TextDontClip,
                                    mChannelLabels[i].Text().c_str());
            }
            if (mShowNumericValues)
                DrawNumericValue(p, legendRect, GetNumericValue(i).c_str());
            legendRect.translate(0, deltaY);
        }
    }
}

void SignalDisplay::DrawValueUnits(const PaintData &p)
{
    PhysicalUnit previousUnit;
    for (int visibleGroup = 0; visibleGroup < mNumDisplayGroups; ++visibleGroup)
    {
        int group = mTopGroup + visibleGroup;
        bool visibleUnit = false;
        int channel = -1;
        for (auto i : mChannelGroups[group])
        {
            if ((i < mValueUnitsVisible.size() && mValueUnitsVisible[i]) || mShowValueUnit)
            {
                visibleUnit = true;
                channel = i;
                break;
            }
        }
        PhysicalUnit unit = mValueUnits.size() > channel ? mValueUnits[channel] : mValueUnit;
        unit.SetRawMax(mMaxValues.size() > channel ? mMaxValues[channel] : mMaxValue);
        unit.SetRawMin(mMinValues.size() > channel ? mMinValues[channel] : mMinValue);
        if (unit == previousUnit && unit.RawMax() == previousUnit.RawMax() && unit.RawMin() == previousUnit.RawMin()) {
            visibleUnit = false;
        }
        else {
            previousUnit = unit;
        }
        if (visibleUnit) {
            DrawValueUnit(p, channel);
        }
    }
}

void SignalDisplay::DrawValueUnit(const PaintData &p, int ch)
{
    float baseInterval = mDataHeight;
    if (mNumDisplayGroups > 0)
        baseInterval /= mNumDisplayGroups;

    if (baseInterval > 0)
    {
        const PhysicalUnit& unit = mValueUnits.size() > ch ? mValueUnits[ch] : mValueUnit;
        double maxValue = mMaxValues.size() > ch ? mMaxValues[ch] : mMaxValue;
        double minValue = mMinValues.size() > ch ? mMinValues[ch] : mMinValue;
        p.painter->setFont(p.labelFont);
        p.painter->setPen(p.backgroundColor);
        p.painter->setBackground(p.markerColor);
        p.painter->setBackgroundMode(Qt::OpaqueMode);
        // Find a rounded value that is close to display range.
        double unitsPerPixel = ::fabs((maxValue - minValue) * unit.Gain() / baseInterval),
               scale = ::pow(10.0, ::ceil(::log10(unitsPerPixel * 0.95 * baseInterval)));
        int percent = 100;
        while (percent * scale / unitsPerPixel >= 95 * baseInterval &&
               percent * scale / unitsPerPixel > 100 * mMarkerHeight)
            percent -= 10;
        double rulerLength = percent * scale / 100;
        int pixelLength = ToInt(rulerLength / unitsPerPixel);

        std::ostringstream oss;
        oss << unit.RawToPhysical(rulerLength / unit.Gain()).Utf8();
        QString label(oss.str().c_str());
        QRect labelRect;
        if (minValue == 0)
        {
            int left = SampleLeft(0) + mTickLength, top = ChannelBottom(ch) - pixelLength;
            labelRect = QRect(left, top, 0, 0);
            p.painter->drawText(labelRect, Qt::AlignTop | Qt::TextSingleLine | Qt::AlignLeft, label, &labelRect);
            QRect lineRect;
            lineRect.setLeft(SampleLeft(0));
            lineRect.setTop(labelRect.top());
            lineRect.setRight(left);
            lineRect.setHeight(1);
            p.painter->fillRect(lineRect, p.markerBrush);
        }
        else
        {
            int left = SampleLeft(0),
                center = (ChannelBottom(ch) + ChannelTop(ch)) / 2;
            labelRect = QRect(left, center, 0, 0);
            p.painter->drawText(labelRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignLeft, label, &labelRect);
            QRect lineRect;
            lineRect.setLeft(labelRect.left());
            lineRect.setTop(center - (pixelLength + p.markerWidth) / 2 - 1);
            lineRect.setRight(labelRect.right());
            lineRect.setBottom(center - (pixelLength - p.markerWidth) / 2 - 1);
            p.painter->fillRect(lineRect, p.markerBrush);
            lineRect.setTop(lineRect.top() + pixelLength);
            lineRect.setBottom(lineRect.bottom() + pixelLength);
            p.painter->fillRect(lineRect, p.markerBrush);
        }
        labelRect.adjust(-1, 0, 1, 0);
        p.painter->fillRect(labelRect, p.markerBrush);
        labelRect.adjust(1, 0, -1, 0);
        p.painter->drawText(labelRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignLeft, label);
    }
}

void SignalDisplay::DrawNumericValue(const PaintData &p, const QRect &inRect, const char *inValue)
{
    p.painter->setFont(p.monoFont);
    int w = p.painter->fontMetrics().boundingRect(inValue).width();
    if (mNumericValueWidth < w)
    {
        mNumericValueWidth = w;
        SyncLabelWidth();
        Invalidate();
    }
    p.painter->drawText(inRect, Qt::AlignVCenter | Qt::TextSingleLine | Qt::AlignRight | Qt::TextDontClip, inValue);
}

void SignalDisplay::DrawSelection(const PaintData& p)
{
    QColor color = mSelectionColor.ToQRgb();
    color.setAlpha(0x7f);
    for (int group = 0; group < mChannelGroups.size(); ++group) {
        bool groupSelected = false;
        for (int idx = 0; idx < mChannelGroups[group].size(); ++idx) {
            int ch = mChannelGroups[group][idx];
            if (ch < mChannelSelected.size() && mChannelSelected[ch]) {
                groupSelected = true;
                break;
            }
        }
        if (groupSelected) {
            float top = GroupTop(group - mTopGroup), height = GroupBottom(group - mTopGroup) - top;
            float left = SampleLeft(0), width = SampleRight(mNumSamples) - left;
            QRectF rect(left, top, width, height);
            p.painter->fillRect(rect, color);
        }
    }
}

void SignalDisplay::CleanupPainting(PaintData &p)
{
    delete p.painter;
}
