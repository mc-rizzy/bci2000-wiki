////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
//  Description: Test cases for Tiny::Url.
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
#include "Url.h"

#include "UnitTest.h"
#include <sstream>

UnitTest(UrlTest)
{
    const char* testCases[] =
    {
      "",
      "http://user:password@host.org:1234/some/path/",
      "http://host.org:1234/some/path/",
      "http://user:password@host.org/some/path/",
      "http://user@host.org/some/path/",
      "http://host.org:1234",
      "http://host.org",
    };
    for (auto pCase : testCases) {
        Url url(pCase);
        std::ostringstream oss;
        oss << url;
        if (oss.str() != pCase)
            TestFail << pCase << " != " << oss.str();
    }
}
