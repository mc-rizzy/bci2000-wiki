////////////////////////////////////////////////////////////////////////////////
// $Id: EnvVariable.h 9216 2026-01-01 14:20:40Z mellinger $
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: A class that encapsulates OS specific access to environment
//   variables, and TempEnvVariable RAAI class that reverts changes to an
//   environment variable when destructed.
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
#ifndef TINY_ENV_VARIABLE_H
#define TINY_ENV_VARIABLE_H

#include <string>

namespace Tiny
{

class EnvVariable
{
  public:
    static bool Set(const std::string &name, const std::string &value);
    static bool Get(const std::string &name, std::string &value);
    static bool Clear(const std::string &name);

    explicit EnvVariable(const std::string &name);

    bool Exists() const;
    // Set()/Get() will return false on error.
    bool Set(const std::string &value);
    bool Get(std::string &) const;
    // returns empty string in case of error.
    std::string Get() const;

  protected:
    std::string mName;
};

class TempEnvVariable : public EnvVariable
{ // unlike EnvVariable, TempEnvVariable resets the variable back to its original state when destructed
  public:
    explicit TempEnvVariable(const std::string& name);
    ~TempEnvVariable();

  private:
    std::string mInitialValue;
    bool mExisted;
};

} // namespace Tiny

using Tiny::EnvVariable;
using Tiny::TempEnvVariable;

#endif // TINY_ENV_VARIABLE_H