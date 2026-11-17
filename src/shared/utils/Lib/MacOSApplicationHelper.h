////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: Objective-C source file using NSWorkspace to launch a
//   child process.
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
#ifndef MACOS_APPLICATION_HELPER_H
#define MACOS_APPLICATION_HELPER_H

#include <string>
#include <vector>
#include <sys/types.h>

namespace Tiny
{

namespace ProcessUtils
{

class MacOSApplicationHelper
{
public:
    static std::pair<pid_t, int> StartApplicationBundle(const std::string& bundlePath, const std::vector<std::string>& args = {});
};

} // namespace ProcessUtils

} // namespace Tiny

#endif // MACOS_APPLICATION_HELPER_H
