////////////////////////////////////////////////////////////////////////////////
// $Id: Rectify.h 9408 2026-06-15 13:14:53Z mellinger $
// Authors: griffin.milsap@gmail.com
// Description: Rectification functions
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
#ifndef RECTIFY_H
#define RECTIFY_H

#include <algorithm>
#include <numeric>

namespace Rectify
{
template <typename T> T HalfWaveRectify(const T &val);
template <typename T> void HalfWaveRectify(const T &, T &);
template <typename T> T FullWaveRectify(const T &val);
template <typename T> void FullWaveRectify(const T &, T &);
} // namespace Rectify

// Implementation
template <typename T> T Rectify::HalfWaveRectify(const T &val)
{
    return (val < 0.0) ? 0.0 : val;
}

template <typename T> void Rectify::HalfWaveRectify(const T &inData, T &outResult)
{
    int count = inData.Channels() * inData.Elements();
    const auto pInput = inData.Data();
    auto pOutput = outResult.MutableData(GenericSignal::intentOverwriteContents);
    for (int i = 0; i < count; ++i)
        pOutput[i] = HalfWaveRectify(pInput[i]);
}

template <typename T> T Rectify::FullWaveRectify(const T &val)
{
    return fabs(val);
}

template <typename T> void Rectify::FullWaveRectify(const T &inData, T &outResult)
{
    int count = inData.Channels() * inData.Elements();
    const auto pInput = inData.ConstData();
    auto pOutput = outResult.MutableData(GenericSignal::intentOverwriteContents);
    for (int i = 0; i < count; ++i)
        pOutput[i] = FullWaveRectify(pInput[i]);
}

#endif // RECTIFY_H
