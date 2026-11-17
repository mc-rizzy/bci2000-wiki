//////////////////////////////////////////////////////////////////////
// $Id: DialogType.h 9216 2026-01-01 14:20:40Z mellinger $
// Authors: hill@neurotechcenter.org
// Description: Scripting extension for prompting the user with
//              customizable dialogs.
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
#ifndef DIALOG_TYPE_H
#define DIALOG_TYPE_H

#include "ScriptingExtensions.h"

namespace ScriptingExtensions {

  class DialogType : public ObjectType
  {
    public:
        const char* Name() const override { return "Dialog"; }
        const char* Help() const override { return "Custom Dialog [title|message|var|buttons ...] [title|message|var|buttons ...] ..."; }
        static bool Custom(std::istream&, std::ostream&, std::ostream&);
    protected:
        const MethodEntry* MethodTable() const override;
  };
}

#endif // DIALOG_TYPE_H

