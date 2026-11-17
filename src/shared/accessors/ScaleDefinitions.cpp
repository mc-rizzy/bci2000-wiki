////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: An EnvironmentExtension that handles the ScaleDefinitions
//   parameter which provides information about scaling from raw to physical
//   values for a subset of BCI2000 states.
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
#include "ScaleDefinitions.h"
#include "BCIException.h"

ScaleDefinitions ScaleDefinitions::sInstance;

void ScaleDefinitions::AddScaleDefinitions(const std::string* pDef, int count)
{
    if (!Parameters->Exists("/ScaleDefinitions")) {
        BEGIN_PARAMETER_DEFINITIONS
            "Source matrix /ScaleDefinitions= 0 [Type Offset Gain] "
            "// Scale definitions for a subset of states (type is one of signed, unsigned, float)(noedit)",
        END_PARAMETER_DEFINITIONS
    }

    MutableParamRef ScaleDefinitions = Parameter("/ScaleDefinitions");
    for (int i = 0; i < count; ++i) {
        ScaleDefinitions->SetNumRows(ScaleDefinitions->NumRows() + 1);
        int idx = ScaleDefinitions->NumRows() - 1;
        std::string name, type, gain;
        double offset;
        std::istringstream iss(pDef[i]);
        iss >> name >> type >> offset >> gain;
        if (!iss) {
            throw bcierr << "Parse error in Scale Definition: '" << pDef[i] << "'";
        }
        if (type != "signed" && type != "unsigned" && type != "float") {
            throw bcierr << "Unknown type in Scale Definition: '" << pDef[i] << "'";
        }
        try {
            PhysicalUnit unit;
            unit.SetOffset(offset).SetGainWithSymbol(gain);
        }
        catch (const bci::Exception& e) {
            throw bcierr << "Error in gain specification: '" << gain << "': " << e.What();
        }
        ScaleDefinitions->RowLabels()[idx] = name;
        ScaleDefinitions(idx, 0) = type;
        ScaleDefinitions(idx, 1) = offset;
        ScaleDefinitions(idx, 2) = gain;
    }
}
