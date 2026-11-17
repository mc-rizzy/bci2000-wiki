////////////////////////////////////////////////////////////////////////////////
// $Id: GenericSignal.cpp 9465 2026-07-18 20:48:20Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: GenericSignal is the BCI2000 type representing filter input and
//              output data.
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
#include "GenericSignal.h"

#include "BinaryData.h"
#include "LengthField.h"
#include "SharedMemoryMap.h"

#include "Exception.h"
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>

static SharedMemoryMap &ShmContainer()
{
    static SharedMemoryMap instance;
    return instance;
}

const GenericSignal::ValueType GenericSignal::NaN = std::numeric_limits<ValueType>::quiet_NaN();

GenericSignal::GenericSignal()
{
    SetProperties(mProperties, true);
}

GenericSignal::~GenericSignal()
{
}

GenericSignal::GenericSignal(const GenericSignal& other)
{
    AssignFrom(other);
}

GenericSignal& GenericSignal::operator=(const GenericSignal& other)
{
    return AssignFrom(other);
}

GenericSignal::GenericSignal(size_t inChannels, size_t inElements, SignalType::Type inType)
{
    SetProperties(SignalProperties(inChannels, inElements, inType), true);
}

GenericSignal::GenericSignal(size_t inChannels, size_t inElements, SignalType inType)
{
    SetProperties(SignalProperties(inChannels, inElements, inType), true);
}

GenericSignal::GenericSignal(size_t inChannels, size_t inElements, ValueType* inpValues)
{
    mProperties = SignalProperties(inChannels, inElements, SignalType::float32);
    auto pMemory = std::make_shared<LazyArray<ValueType>::ForeignMemory>(inpValues, inChannels * inElements);
    mValues = LazyArray<ValueType>(pMemory);
    mForeignData = true;
}

GenericSignal::GenericSignal(const SignalProperties &inProperties)
{
    SetProperties(inProperties, true);
}

GenericSignal::GenericSignal(const SignalProperties &inProperties, ValueType inValue)
{
    SetProperties(inProperties, true, inValue);
}

GenericSignal& GenericSignal::SetProperties(const SignalProperties& inSp)
{
    return SetProperties(inSp, true);
}

GenericSignal& GenericSignal::SetProperties(const SignalProperties& inSp, bool inInitialize, const ValueType& inInitializer)
{
    if (inSp.Channels() != mProperties.Channels() || inSp.Elements() != mProperties.Elements())
    {
        if (mForeignData)
            throw std_runtime_error << "Trying to resize a signal that was created from foreign data";

        size_t newSize = inSp.Channels() * inSp.Elements();
        LazyArray<ValueType> newValues;
        if (mSharedMemory)
        {
            if (mSharedMemory->Protocol() == "file://")
                throw std_runtime_error << "Cannot resize shared memory if tied to a file";
            mSharedMemory = ShmContainer().New(newSize * sizeof(ValueType));
            std::shared_ptr<LazyArray<ValueType>::Memory> p(new LazyArray<ValueType>::ShmMemory(mSharedMemory, newSize));
            newValues = LazyArray<ValueType>(p);
        }
        else
        {
            newValues = LazyArray<ValueType>(newSize);
        }
        ValueType* pNewValues = newValues.Data();
        const ValueType* pValues = mValues.Data();
        for (int ch = 0; ch < std::min(mProperties.Channels(), inSp.Channels()); ++ch)
        {
            for (int el = 0; el < std::min(mProperties.Elements(), inSp.Elements()); ++el)
                pNewValues[inSp.LinearIndex(ch, el)] = pValues[mProperties.LinearIndex(ch, el)];
            if (inInitialize && inSp.Elements() > mProperties.Elements()) {
                if (inInitializer == 0) {
                    ::memset(pNewValues + inSp.LinearIndex(ch, mProperties.Elements()), 0, (inSp.Elements() - mProperties.Elements()) * sizeof(ValueType));
                }
                else {
                    for (int el = mProperties.Elements(); el < inSp.Elements(); ++el)
                        pNewValues[inSp.LinearIndex(ch, el)] = inInitializer;
                }
            }
        }
        if (inInitialize) {
            if (inInitializer == 0 && inSp.Elements() > 0) {
                for (int ch = mProperties.Channels(); ch < inSp.Channels(); ++ch)
                    ::memset(pNewValues + inSp.LinearIndex(ch, 0), 0, inSp.Elements() * sizeof(ValueType));
            }
            else {
                for (int ch = mProperties.Channels(); ch < inSp.Channels(); ++ch)
                    for (int el = 0; el < inSp.Elements(); ++el)
                        pNewValues[inSp.LinearIndex(ch, el)] = inInitializer;
            }
        }
        mValues = newValues;
    }
    mProperties = inSp;
    return *this;
}

GenericSignal &GenericSignal::SetAllValues(GenericSignal::ValueType value)
{
    GenericSignal::ValueType *pData = mValues.Data();
    for (int i = 0; i < mValues.Count(); ++i)
        *pData++ = value;
    return *this;
}

std::ostream &GenericSignal::InsertInto(std::ostream &os) const
{
    std::string indent(static_cast<size_t>(os.width()), ' ');

    os << '\n' << indent << "SignalProperties { ";
    mProperties.InsertInto(os);
    os << '\n' << indent << "}";
    if (mSharedMemory)
        os << '\n' << indent << "SharedMemory { " << mSharedMemory->Name() << '\n' << indent << "}";
    os << std::setprecision(7);
    for (int j = 0; j < Elements(); ++j)
    {
        os << '\n' << indent;
        for (int i = 0; i < Channels(); ++i)
        {
            os << std::setw(14) << Value(i, j) << ' ';
        }
    }
    return os;
}

std::ostream &GenericSignal::Serialize(std::ostream &os) const
{
    SignalType type = Type();
    type.SetShared(!!mSharedMemory);
    type.Serialize(os);
    LengthField<2> channelsField(Channels()), elementsField(Elements());
    channelsField.Serialize(os);
    elementsField.Serialize(os);
    if (type.Shared())
    {
        std::atomic_thread_fence(std::memory_order_seq_cst);
        os.write(mSharedMemory->Name().c_str(), mSharedMemory->Name().length() + 1);
    }
    else
    {
        const ValueType* pValues = mValues.Data();
        for (int i = 0; i < Channels(); ++i)
            for (int j = 0; j < Elements(); ++j)
                WriteValueBinary(os, pValues + LinearIndex(i, j));

    }
    return os;
}

std::istream &GenericSignal::Unserialize(std::istream &is)
{
    SignalType type;
    LengthField<2> channels, elements;
    type.Unserialize(is);
    channels.Unserialize(is);
    elements.Unserialize(is);
    bool shared = type.Shared();
    type.SetShared(false);
    SetProperties(SignalProperties(channels, elements, type));
    if (shared)
    {
        std::string name;
        std::getline(is, name, '\0');
        AttachToSharedMemory(name);
        std::atomic_thread_fence(std::memory_order_seq_cst);
    }
    else
    {
        ValueType* pValues = mValues.Data();
        for (int i = 0; i < Channels(); ++i)
            for (int j = 0; j < Elements(); ++j)
                ReadValueBinary(is, pValues + LinearIndex(i, j));
    }
    return is;
}

std::ostream &GenericSignal::WriteValueBinary(std::ostream &os, const ValueType* pValue) const
{
    switch (Type())
    {
    case SignalType::int16:
        BinaryData<int16_t, LittleEndian>(*pValue).Put(os);
        break;

    case SignalType::float24:
        PutValue_float24(os, *pValue);
        break;

    case SignalType::float32:
        BinaryData<float, LittleEndian>(*pValue).Put(os);
        break;

    case SignalType::int32:
        BinaryData<int32_t, LittleEndian>(*pValue).Put(os);
        break;

    default:
        os.setstate(os.failbit);
    }
    return os;
}

std::istream &GenericSignal::ReadValueBinary(std::istream &is, ValueType* pValue)
{
    switch (Type())
    {
    case SignalType::int16:
        *pValue = BinaryData<int16_t, LittleEndian>(is);
        break;

    case SignalType::float24:
        *pValue = GetValue_float24(is);
        break;

    case SignalType::float32:
        *pValue = BinaryData<float, LittleEndian>(is);
        break;

    case SignalType::int32:
        *pValue = BinaryData<int32_t, LittleEndian>(is);
        break;

    default:
        is.setstate(is.failbit);
    }
    return is;
}

void GenericSignal::PutValue_float24(std::ostream &os, ValueType value)
{
    int mantissa, exponent;
    if (value == 0.0)
    {
        mantissa = 0;
        exponent = 1;
    }
    else
    {
        exponent = static_cast<int>(::ceil(::log10(::fabs(value))));
        mantissa = static_cast<int>(value / ::pow(10.0, exponent)) * 10000;
        exponent -= 4;
    }
    os.put(mantissa & 0xff).put(mantissa >> 8);
    os.put(exponent & 0xff);
}

GenericSignal::ValueType GenericSignal::GetValue_float24(std::istream &is)
{
    signed short mantissa = is.get();
    mantissa |= is.get() << 8;
    signed char exponent = is.get();
    return mantissa * ::pow(10.0, exponent);
}

GenericSignal &GenericSignal::AssignFrom(const GenericSignal &s)
{
    SetProperties(s.Properties(), false);
    AssignValues(s);
    return *this;
}

GenericSignal &GenericSignal::AssignValues(const GenericSignal &s)
{
    bool mismatch = s.Channels() != Channels() || s.Elements() != Elements();
    if (mismatch)
        SetProperties(SignalProperties(s.Channels(), s.Elements()), false);

    if (!mSharedMemory && !s.mSharedMemory && !mForeignData &&!s.mForeignData)
        mValues.ShallowAssignFrom(s.mValues);
    else
        mValues.DeepAssignFrom(s.mValues);
    return *this;
}

GenericSignal::ValueType* GenericSignal::MutableData(int intent)
{
    return mValues.GetWritableData(intent == intentRetainContents);
}

const GenericSignal::ValueType* GenericSignal::ConstData() const
{
    return mValues.Data();
}

size_t GenericSignal::LinearIndex(size_t ch, size_t el) const
{
    return mProperties.LinearIndex(ch, el);
}

LazyArray<GenericSignal::ValueType>::MemoryObj GenericSignal::MemoryRef() const
{
    return mValues.MemoryRef();
}

void GenericSignal::EnsureDeepCopy()
{
    mValues.GetWritableData(true);
}

bool GenericSignal::ShareAcrossModules()
{
    if (!mSharedMemory && mValues.Count() != 0)
    {
        mSharedMemory = ShmContainer().New(mValues.Count() * sizeof(ValueType));
        std::shared_ptr<LazyArray<ValueType>::Memory>
            p(new LazyArray<ValueType>::ShmMemory(mSharedMemory, mValues.Count()));
        LazyArray<ValueType> newValues(p);
        newValues.DeepAssignFrom(mValues);
        mValues.ShallowAssignFrom(newValues);
    }
    return !!mSharedMemory;
}

void GenericSignal::AttachToSharedMemory(const std::string &inName)
{
    if (!mSharedMemory || mSharedMemory->Name() != inName)
    {
        mSharedMemory = ShmContainer().Get(inName);
        std::shared_ptr<LazyArray<ValueType>::Memory> p(new LazyArray<ValueType>::ShmMemory(mSharedMemory, mValues.Count()));
        mValues = LazyArray<ValueType>(p);
    }
}

// GenericChannel
GenericChannel &GenericChannel::operator=(const GenericChannel &inChannel)
{
    for (size_t i = 0; i < size(); i++)
        (*this)[i] = inChannel[i];
    return (*this);
}

// GenericElement
GenericElement &GenericElement::operator=(const GenericElement &inElement)
{
    for (size_t i = 0; i < size(); i++)
        (*this)[i] = inElement[i];
    return (*this);
}
