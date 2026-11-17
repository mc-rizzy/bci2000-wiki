////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: An EnvironmentExtension that handles the ScaleDefinitions
//   parameter which provides information about scaling from raw to physical
//   values for a subset of BCI2000 states.
//   The format of a scale definition is 
//     <name> <type> <offset> <gain>
//   where <name> is the state name, <type> is one of signed/unsigned/float,
//   offset is an offset to subtract prior to multiplication with <gain>.
//   Any physical unit (such as V, A, kg) is supposed to be part of the
//   <gain> field.
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
#ifndef SCALE_DEFINITIONS_H
#define SCALE_DEFINITIONS_H

#include "Environment.h"
#include <string>

#define BEGIN_SCALE_DEFINITIONS                                                                                        \
    {                                                                                                                  \
        const std::string def_[] = {
#define END_SCALE_DEFINITIONS                                                                                          \
        };                                                                                                             \
        AddScaleDefinitions(def_, sizeof(def_) / sizeof(*def_));                                                       \
    }

class ScaleDefinitions : public EnvironmentExtension
{
public:
    void Publish() override {}
    void Preflight() const override {}
    void Initialize() override {}

    void AddScaleDefinitions(const std::string*, int);

    static void AddScaleDefinitions_(const std::string* pDef, int count)
    {
        sInstance.AddScaleDefinitions(pDef, count);
    }

private:
    static ScaleDefinitions sInstance;
};

inline void AddScaleDefinitions(const std::string* pDef, int count)
{
    ScaleDefinitions::AddScaleDefinitions_(pDef, count);
}

#endif // SCALE_DEFINITIONS_H
