////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A filter container for filters applied to the signal immediately
//   after it has been output by the GenericADC source.
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
#include "SourceFilterChain.h"

SourceFilterChain::SourceFilterChain()
{
}

SourceFilterChain::~SourceFilterChain()
{
    for (auto pFilter : mFilters)
        delete pFilter;
}

void SourceFilterChain::AddFilter(GenericFilter* pFilter)
{
    mFilters.push_back(pFilter);
    mSignals.resize(mFilters.size() - 1);
}

void SourceFilterChain::Publish()
{
    for (auto pFilter : mFilters)
        pFilter->CallPublish();
}

void SourceFilterChain::AutoConfigAndPreflight(const SignalProperties& Input, SignalProperties& Output)
{
    if (mFilters.empty()) {
        Output = Input;
    }
    else {
        for (int i = 0; i < mFilters.size(); ++i) {
            const SignalProperties& input = (i == 0) ? Input : mSignals[i - 1].Properties();
            SignalProperties output = input;
            mFilters[i]->CallAutoConfig(input);
            mFilters[i]->CallPreflight(input, output);
            if (i < mSignals.size()) {
                mSignals[i].SetProperties(output);
            }
            else {
                Output = output;
            }
        }
    }
}

void SourceFilterChain::Initialize(const SignalProperties& Input, const SignalProperties& Output)
{
    for (int i = 0; i < mFilters.size(); ++i) {
        const SignalProperties& input = (i == 0) ? Input : mSignals[i - 1].Properties();
        const SignalProperties& output = (i < mSignals.size()) ? mSignals[i].Properties() : Output;
        mFilters[i]->CallInitialize(input, output);
    }
}

void SourceFilterChain::StartRun()
{
    for (auto pFilter : mFilters)
        pFilter->CallStartRun();
}

void SourceFilterChain::StopRun()
{
    for (auto pFilter : mFilters)
        pFilter->CallStopRun();
}

void SourceFilterChain::Process(const GenericSignal& Input, GenericSignal& Output)
{
    if (mFilters.empty()) {
        Output = Input;
    }
    else {
        for (int i = 0; i < mFilters.size(); ++i) {
            const GenericSignal& input = (i == 0) ? Input : mSignals[i - 1];
            GenericSignal& output = (i < mSignals.size()) ? mSignals[i] : Output;
            mFilters[i]->CallProcess(input, output);
        }
    }
}

void SourceFilterChain::Halt()
{
    for (auto pFilter : mFilters)
        pFilter->CallHalt();
}
