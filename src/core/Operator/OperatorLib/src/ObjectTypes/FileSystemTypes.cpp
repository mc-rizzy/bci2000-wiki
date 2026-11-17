////////////////////////////////////////////////////////////////////////////////
// $Id: FileSystemTypes.cpp 9216 2026-01-01 14:20:40Z mellinger $
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: CommandInterpreter types for files and directories.
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
#include "FileSystemTypes.h"

#include "BCIException.h"
#include "CommandInterpreter.h"
#include "FileUtils.h"
#include "ProcessUtils.h"
#include "WildcardMatch.h"

#define MATCHES(token, X) (!::stricmp(token.c_str(), X))

namespace Interpreter
{

// DirectoryType
DirectoryType DirectoryType::sInstance;
const ObjectType::MethodEntry DirectoryType::sMethodTable[] = {METHOD(Extract),     {"Get", &Extract},
                                                               METHOD(Parent),      METHOD(Exists),
                                                               {"Is", &Exists},     METHOD(Current),
                                                               METHOD(Change),      METHOD(Make),
                                                               {"Create", &Make},   METHOD(List),
                                                               METHOD(Rename),      METHOD(Remove),
                                                               METHOD(ForceRemove), END};

bool DirectoryType::Extract(CommandInterpreter &inInterpreter)
{
    inInterpreter.Out() << FileUtils::ExtractDirectoryS(inInterpreter.GetToken());
    return true;
}

bool DirectoryType::Parent(CommandInterpreter &inInterpreter)
{
    inInterpreter.Out() << FileUtils::ParentDirectoryS(inInterpreter.GetToken());
    return true;
}

bool DirectoryType::Exists(CommandInterpreter &inInterpreter)
{
    bool result = FileUtils::IsDirectory(inInterpreter.GetToken());
    inInterpreter.Out() << (result ? "true" : "false");
    return true;
}

bool DirectoryType::Current(CommandInterpreter &inInterpreter)
{
    inInterpreter.Out() << FileUtils::WorkingDirectoryS();
    return true;
}

bool DirectoryType::Change(CommandInterpreter &inInterpreter)
{
    std::string dir = inInterpreter.GetToken();
    if (!FileUtils::ChangeDirectory(dir))
        inInterpreter.Out() << "Could not change current directory to \"" << dir << "\"";
    return true;
}

bool DirectoryType::Make(CommandInterpreter &inInterpreter)
{
    std::string dir = inInterpreter.GetToken();
    if (!FileUtils::MakeDirectory(dir))
        inInterpreter.Out() << "Could not make directory \"" << dir << "\"";
    return true;
}

bool DirectoryType::Rename(CommandInterpreter &inInterpreter)
{
    std::string dir = inInterpreter.GetToken(), newName = inInterpreter.GetToken();
    if (!FileUtils::IsDirectory(dir))
        throw bciexception << "There is no directory named \"" << dir << "\"";
    if (!FileUtils::Rename(dir, newName))
        throw bciexception << "Could not rename \"" << dir << "\" to \"" << newName << "\"";
    return true;
}

bool DirectoryType::Remove(CommandInterpreter &inInterpreter)
{
    std::string dir = inInterpreter.GetToken();
    if (!FileUtils::IsDirectory(dir))
        throw bciexception << "There is no directory named \"" << dir << "\"";
    if (!FileUtils::RemoveDirectory(dir, false))
        throw bciexception << "Could not remove directory \"" << dir << "\"";
    return true;
}

bool DirectoryType::ForceRemove(CommandInterpreter &inInterpreter)
{
    std::string dir = inInterpreter.GetToken();
    if (!FileUtils::IsDirectory(dir))
        throw bciexception << "There is no directory named \"" << dir << "\"";
    if (!FileUtils::RemoveDirectory(dir, true))
        throw bciexception << "Could not remove directory \"" << dir << "\"";
    return true;
}

bool DirectoryType::List(CommandInterpreter &inInterpreter)
{
    std::string args;
#if _WIN32
    args = "/n ";
#else
    args = "-l ";
#endif
    std::string remainder = inInterpreter.GetRemainingTokens();
    args += remainder;
    inInterpreter.Out() << ListDirectory(args);
    return true;
}

std::string DirectoryType::ListDirectory(const std::string &inArgs)
{
    std::string command;
#if _WIN32
    command = "dir " + inArgs;
#else
    command = "ls " + inArgs;
#endif
    int exitCode = 0;
    std::stringstream oss;
    if (!ProcessUtils::SystemExecuteSynchronously(command, oss, exitCode))
        throw bciexception << "Could not get directory listing";
#if _WIN32
    std::string listing;
    char c;
    while (oss.get(c))
        if (c != '\r')
            listing += c;
    return listing;
#else  // _WIN32
    return oss.str();
#endif // _WIN32
}

bool DirectoryType::ListSelection(CommandInterpreter &inInterpreter, const std::string &inDir,
                                  const std::string &inWildcard, bool (*inSelector)(const std::string &))
{
    std::string dir = inDir.empty() ? "." : inDir, wildcard = inWildcard.empty() ? "*" : inWildcard;
    dir = FileUtils::NormalizedPath(dir);
    FileUtils::List list;
    if (!FileUtils::ListDirectory(dir, list))
        throw bciexception << "Could not list directory \"" << dir << "\"";
    for (size_t i = 0; i < list.size(); ++i)
        if (WildcardMatch(wildcard, list[i], false) && inSelector(FileUtils::EnsureSeparator(dir) + list[i]))
            inInterpreter.Out() << list[i] << '\n';
    return true;
}

// DirectoriesType
DirectoriesType DirectoriesType::sInstance;
const ObjectType::MethodEntry DirectoriesType::sMethodTable[] = {METHOD(List), END};

bool DirectoriesType::List(CommandInterpreter &inInterpreter)
{
    std::string dir = inInterpreter.GetOptionalToken(), wildcard = inInterpreter.GetOptionalToken();
    return DirectoryType::ListSelection(inInterpreter, dir, wildcard, &FileUtils::IsDirectory);
}

// FileType
FileType FileType::sInstance;
const ObjectType::MethodEntry FileType::sMethodTable[] = {
    METHOD(Extract), {"Get", &Extract}, METHOD(Exists), {"Is", &Exists},     METHOD(List),
    METHOD(Rename),  {"Move", &Rename}, METHOD(Remove), {"Delete", &Remove},
    METHOD(Read), METHOD(Write), {"Overwrite", &Write}, METHOD(Append),
    END};

bool FileType::Extract(CommandInterpreter &inInterpreter)
{
    std::string file = inInterpreter.GetToken();
    if (!::stricmp(file.c_str(), "Base"))
        inInterpreter.Out() << FileUtils::ExtractBase(inInterpreter.GetToken());
    else
        inInterpreter.Out() << FileUtils::ExtractFile(file);
    return true;
}

bool FileType::Read( CommandInterpreter &inInterpreter )
{
    std::string filename = inInterpreter.GetToken();
    std::string canonical = FileUtils::CanonicalPath(filename);
    if (!FileUtils::IsFile(canonical))
        throw bciexception << "There is no file named \"" << filename << "\"";
    std::ifstream file(canonical, std::ios::in | std::ios::binary);
    if (!file) throw bciexception << "File \"" << filename << "\" exists, but could not be opened for reading";
    std::ostringstream contents;
    contents << file.rdbuf();
    std::string text = contents.str();
    for (size_t pos = 0; (pos = text.find("\r\n", pos)) != std::string::npos; pos++) text.replace(pos, 2, "\n");  // CRLF -> LF
    for (size_t pos = 0; (pos = text.find('\r',   pos)) != std::string::npos; pos++) text[pos] = '\n';            // remaining CR -> LF
    inInterpreter.Out() << text;
    return true;
}
bool FileType::WriteToFile( CommandInterpreter &inInterpreter, bool append )
{
    bool forceLineEnding = true;
    std::string filename = inInterpreter.GetToken();
    std::string content = inInterpreter.GetToken();
    std::string option = inInterpreter.GetOptionalToken();
    if (option.length())
    {
        if(!MATCHES(option, "without")) throw bciexception << "Unexpected token \"" << option << "\"";
        option = inInterpreter.GetRemainingTokens();
        for (size_t pos = 0; (pos = option.find('-',   pos)) != std::string::npos; pos++) option.replace(pos, 1, "");
        for (size_t pos = 0; (pos = option.find(' ',   pos)) != std::string::npos; pos++) option.replace(pos, 1, "");
        if(!MATCHES(option, "lineending") && !MATCHES(option, "lineend") && !MATCHES(option, "linefeed") && !MATCHES(option, "lf") && !MATCHES(option, "crlf") && !MATCHES(option, "cr"))
            throw bciexception << "Unrecognized option \"" << option << "\"";
        forceLineEnding = false;
    }
    if (forceLineEnding && (!content.length() || content[content.length() - 1] != '\n')) content += '\n';
    std::string canonical = FileUtils::AbsolutePath(filename);
    std::ios::openmode mode = std::ios::out;
    if (append) mode |= std::ios::app;
    std::ofstream file(canonical, mode);
    if (!file) throw bciexception << "Failed to open file \"" << filename << "\" for writing";
    file << content;
    if (!file.good()) throw bciexception << "Opened file \"" << filename << "\" but failed to write to it";
    return true;
}

bool FileType::Write( CommandInterpreter &inInterpreter )
{
    return WriteToFile( inInterpreter, false );
}
bool FileType::Append( CommandInterpreter &inInterpreter )
{
    return WriteToFile( inInterpreter, true );
}

bool FileType::Exists(CommandInterpreter &inInterpreter)
{
    bool result = FileUtils::IsFile(inInterpreter.GetToken());
    inInterpreter.Out() << (result ? "true" : "false");
    return true;
}

bool FileType::List(CommandInterpreter &inInterpreter)
{
    return DirectoryType::ListSelection(inInterpreter, ".", inInterpreter.GetToken(), &FileUtils::IsFile);
}

bool FileType::Remove(CommandInterpreter &inInterpreter)
{
    std::string file = inInterpreter.GetToken();
    if (!FileUtils::IsFile(file))
        throw bciexception << "There is no file named \"" << file << "\"";
    if (!FileUtils::RemoveFile(file))
        throw bciexception << "Could not remove file \"" << file << "\"";
    return true;
}

bool FileType::Rename(CommandInterpreter &inInterpreter)
{
    std::string file = inInterpreter.GetToken(), newName = inInterpreter.GetToken();
    if (!FileUtils::IsFile(file))
        throw bciexception << "There is no file named \"" << file << "\"";
    if (!FileUtils::Rename(file, newName))
        throw bciexception << "Could not rename file \"" << file << "\" to \"" << newName << "\"";
    return true;
}

// FilesType
FilesType FilesType::sInstance;
const ObjectType::MethodEntry FilesType::sMethodTable[] = {METHOD(List), END};

bool FilesType::List(CommandInterpreter &inInterpreter)
{
    std::string dir = inInterpreter.GetOptionalToken(), wildcard = inInterpreter.GetOptionalToken();
    return DirectoryType::ListSelection(inInterpreter, dir, wildcard, &FileUtils::IsFile);
}

// PathType
PathType PathType::sInstance;
const ObjectType::MethodEntry PathType::sMethodTable[] = {METHOD(Canonicalize), {"Canonical", &Canonicalize},
                                                          METHOD(Normalize),    {"Normalized", &Normalize},
                                                          METHOD(Real),         METHOD(Exists),
                                                          {"Is", &Exists},      END};

bool PathType::Canonicalize(CommandInterpreter &inInterpreter)
{
    inInterpreter.Out() << FileUtils::CanonicalPath(inInterpreter.GetToken());
    return true;
}

bool PathType::Normalize(CommandInterpreter &inInterpreter)
{
    inInterpreter.Out() << FileUtils::NormalizedPath(inInterpreter.GetToken());
    return true;
}

bool PathType::Real(CommandInterpreter &inInterpreter)
{
    inInterpreter.Out() << FileUtils::RealPath(inInterpreter.GetToken());
    return true;
}

bool PathType::Exists(CommandInterpreter &inInterpreter)
{
    std::string path = inInterpreter.GetToken();
    bool result = FileUtils::IsFile(path) || FileUtils::IsDirectory(path);
    inInterpreter.Out() << (result ? "true" : "false");
    return true;
}

} // namespace Interpreter
