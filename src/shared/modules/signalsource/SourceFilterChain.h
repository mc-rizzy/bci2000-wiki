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
#ifndef SOURCE_FILTER_CHAIN_H
#define SOURCE_FILTER_CHAIN_H

#include "GenericFilter.h"

class SourceFilterChain
{
public:
    SourceFilterChain();
    ~SourceFilterChain();

    SourceFilterChain(const SourceFilterChain&) = delete;
    SourceFilterChain& operator=(const SourceFilterChain&) = delete;

    void AddFilter(GenericFilter*);
    void Publish();
    void AutoConfigAndPreflight(const SignalProperties&, SignalProperties&);
    void Initialize(const SignalProperties&, const SignalProperties&);
    void StartRun();
    void StopRun();
    void Process(const GenericSignal&, GenericSignal&);
    void Halt();

private:
    std::vector<GenericFilter*> mFilters;
    std::vector<GenericSignal> mSignals;
};

#endif // SOURCE_FILTER_CHAIN_H
