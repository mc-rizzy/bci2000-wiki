////////////////////////////////////////////////////////////////////////////////
// $Id: VariableTypes.cpp 9216 2026-01-01 14:20:40Z mellinger $
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: Local and environment variable object types for the script
//   interpreter.
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
#include "VariableTypes.h"

#include "BCIException.h"
#include "CommandInterpreter.h"
#include "EnvVariable.h"
#include "FileUtils.h"


#define MATCHES(token, X) (!::stricmp(token.c_str(), X))
#define REPLACE(STRING, TARGET, REPLACEMENT) \
    for (size_t startPosition = 0, targetLength = std::string(TARGET).length(); \
         (startPosition = STRING.find(TARGET, startPosition)) != std::string::npos; \
         startPosition++) STRING.replace(startPosition, targetLength, REPLACEMENT);

namespace Interpreter
{

// VariableType
VariableType VariableType::sInstance;
const ObjectType::MethodEntry VariableType::sMethodTable[] = {
    METHOD(Set), METHOD(Get), METHOD(Clear), {"Drop", &Clear}, END};

bool VariableType::Set(CommandInterpreter &inInterpreter)
{
    std::string name = inInterpreter.GetToken(), value = inInterpreter.GetToken();
    inInterpreter.LocalVariables()[name] = value;
    if (name == inInterpreter.ResultName())
        inInterpreter.Out() << value;
    return true;
}

bool VariableType::Get(CommandInterpreter &inInterpreter)
{
    std::string name = inInterpreter.GetToken();
    if (inInterpreter.LocalVariables().Exists(name))
        inInterpreter.Out() << inInterpreter.LocalVariables()[name];
    return true;
}

bool VariableType::Clear(CommandInterpreter &inInterpreter)
{
    std::string name = inInterpreter.GetToken();
    if (inInterpreter.LocalVariables().Exists(name))
        inInterpreter.LocalVariables().erase(name);
    return true;
}

// EnvironmentType
EnvironmentType EnvironmentType::sInstance;
const ObjectType::MethodEntry EnvironmentType::sMethodTable[] = {
    METHOD(Set), METHOD(Get), METHOD(Clear), {"Drop", &Clear},
    METHOD(Write), METHOD(Append), END};

bool EnvironmentType::Set(CommandInterpreter &inInterpreter)
{
    std::string name = inInterpreter.GetToken(), value = inInterpreter.GetToken();
    if (!EnvVariable::Set(name, value))
        throw bciexception << "Could not set environment variable \"" << name << "\"";
    return true;
}

bool EnvironmentType::Get(CommandInterpreter &inInterpreter)
{
    std::string name = inInterpreter.GetToken(), value;
    if (EnvVariable::Get(name, value))
        inInterpreter.Out() << value;
    return true;
}

bool EnvironmentType::Clear(CommandInterpreter &inInterpreter)
{
    std::string name = inInterpreter.GetToken();
    if (!EnvVariable::Clear(name))
        throw bciexception << "Could not clear environment variable \"" << name << "\"";
    return true;
}

bool EnvironmentType::WriteToFile( CommandInterpreter &inInterpreter, bool append )
{
    std::string filename = inInterpreter.GetToken();
    if( MATCHES( filename, "to"     ) ) filename = inInterpreter.GetToken();
    if( MATCHES( filename, "script" ) ) filename = inInterpreter.GetToken();
    if( MATCHES( filename, "file"   ) ) filename = inInterpreter.GetToken();
    std::vector<std::string> names;
    std::stringstream ss; ss.str(inInterpreter.GetRemainingTokens());
	while(!ss.fail()) { std::string name; ss >> name; if(ss.fail()) break; else names.push_back(name); }
	
    if (!names.size()) throw bciexception << "expected one or more variable names after the filename";
    for (auto name : names)
        if(!name.length()) throw bciexception << "empty variable name";
    std::string canonical = FileUtils::AbsolutePath(filename);
    std::ios::openmode mode = std::ios::out;
    if (append) mode |= std::ios::app;
    std::ofstream file(canonical, mode);
    if (!file) throw bciexception << "Failed to open file \"" << filename << "\" for writing";
    for (auto name : names)
    {
        std::string value;
        if (inInterpreter.LocalVariables().Exists(name)) value = inInterpreter.LocalVariables()[name];
        else EnvVariable::Get(name, value);
        REPLACE(name,  "%", "%25"); REPLACE(name,  "$", "%24"); REPLACE(name,  "\"", "%22"); 
        REPLACE(value, "%", "%25"); REPLACE(value, "$", "%24"); REPLACE(value, "\"", "%22");
        file << "set environment  \"" << name << "\"  \"" << value << "\"\n";
        if (!file.good()) throw bciexception << "Opened file \"" << filename << "\" but failed to write to it";
    }
    return true;
}
bool EnvironmentType::Write( CommandInterpreter &inInterpreter )
{
    return WriteToFile( inInterpreter, false );
}
bool EnvironmentType::Append( CommandInterpreter &inInterpreter )
{
    return WriteToFile( inInterpreter, true );
}

} // namespace Interpreter
