//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.de
// Description: Scripting extension dealing with named pipe servers
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
///////////////////////////////////////////////////////////////////////
#include "NamedPipeType.h"

#include "BCI_OperatorLib.h"
#include "ParserToken.h"

namespace {
    ScriptingExtensions::NamedPipeType sInstance;
    const char* sHelp = "Start Pipe <name>, Stop Pipe <name>";
}

namespace ScriptingExtensions {
    const ObjectType::MethodEntry NamedPipeType::sMethodTable[] = {
      {"Start", &NamedPipeType::Start },
      {"Stop", &NamedPipeType::Stop },
      { 0 }
    };

    const char* NamedPipeType::Name() const
    {
        return "Pipe";
    }

    const char* NamedPipeType::Help() const
    {
        return sHelp;
    }

    const ObjectType::MethodEntry* NamedPipeType::MethodTable() const
    {
        return sMethodTable;
    }

    bool NamedPipeType::Start(std::istream& in, std::ostream& out, std::ostream& err)
    {
        ParserToken name;
        if (in >> name)
        {
            int ok = BCI_PipeListen(name.c_str());
            if (!ok)
                err << "Could not start named pipe server at " << name.c_str();
        }
        return true;
    }

    bool NamedPipeType::Stop(std::istream& in, std::ostream& out, std::ostream& err)
    {
        ParserToken name;
        if (in >> name)
        {
            int ok = BCI_PipeCloseOne(name.c_str());
            if (!ok)
                err << "Could not stop named pipe server at " << name.c_str();
        }
        else
        {
            int ok = BCI_PipeClose();
            if (!ok)
                err << "Could not stop all named pipe servers";
        }
        return true;
    }
}
