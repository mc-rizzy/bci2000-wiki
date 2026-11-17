//////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: A class that parses URLs.
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
#ifndef URL_H
#define URL_H

#include <string>
#include <iostream>

namespace Tiny {

class Url
{
public:
  explicit Url(const std::string&);

  bool IsValid() const { return !mProtocol.empty(); }
  const std::string& Protocol() const { return mProtocol; }
  const std::string& Host() const { return mHost; }
  const std::string& Port() const { return mPort; }
  const std::string& User() const { return mUser; }
  const std::string& Password() const { return mPassword; }
  const std::string& Path() const { return mPath; }

  std::ostream& Print(std::ostream&) const;

private:
  std::string mProtocol, mHost, mPort, mUser, mPassword, mPath;
};

} // namespace Tiny

inline std::ostream& operator<<(std::ostream& os, const Tiny::Url& url) { return url.Print(os); }

using Tiny::Url;

#endif // URL_H

