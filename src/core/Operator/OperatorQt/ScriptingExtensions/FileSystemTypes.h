//////////////////////////////////////////////////////////////////////
// $Id$
// Authors: hill@neurotechcenter.org
// Description: Scripting extension for prompting the user to select
//              files and/or directories.
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
#ifndef FILESYSTEM_TYPES_H
#define FILESYSTEM_TYPES_H

#include "ScriptingExtensions.h"

namespace ScriptingExtensions {

  class OutputType : public ObjectType
  {
    public:
        const char* Name() const override { return "Output"; }
        const char* Help() const override { return "Choose Output File[s] [[of] type[s] <pattern> [<pattern2> [...]] ] [[in] directory <dir>] [[with] prompt <msg>]"; }
        static bool Choose(std::istream&, std::ostream&, std::ostream&);
    protected:
        const MethodEntry* MethodTable() const override;
  };
  class InputType : public ObjectType
  {
    public:
        const char* Name() const override { return "Input"; }
        const char* Help() const override { return "Choose [Input] File[s] [[of] type[s] <pattern> [<pattern2> [...]] ] [[from] directory <dir>] [[with] prompt <msg>]"; }
        static bool Choose(std::istream&, std::ostream&, std::ostream&);
    protected:
        const MethodEntry* MethodTable() const override;
  };
  class FileType : public ObjectType
  {
    public:
        const char* Name() const override { return "File"; }
        const char* Help() const override { return nullptr; }
        static bool Choose(std::istream&, std::ostream&, std::ostream&);
    protected:
        const MethodEntry* MethodTable() const override;
  };
  class FilesType : public ObjectType
  {
    public:
        const char* Name() const override { return "Files"; }
        const char* Help() const override { return nullptr; }
        static bool Choose(std::istream&, std::ostream&, std::ostream&);
    protected:
        const MethodEntry* MethodTable() const override;
  };
  class DirectoryType : public ObjectType
  {
    public:
        const char* Name() const override { return "Directory"; }
        const char* Help() const override { return "Choose Directory [[in] directory <dir>] [[with] prompt <msg>]"; }
        static bool Choose(std::istream&, std::ostream&, std::ostream&);
    protected:
        const MethodEntry* MethodTable() const override;
  };
}

#endif // FILESYSTEM_TYPES_H

