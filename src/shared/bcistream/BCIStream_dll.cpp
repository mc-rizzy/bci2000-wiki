////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: Implementation of bcierr and bciout message handlers for a
//   dynamically linked library.
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
#if _WIN32
#include <Windows.h>
#endif // _WIN32

#include "BCIStream.h"
#include "FileUtils.h"
#include <iostream>

// No redirection from stdio, would lead to infinite recursion in cmdline clients
bool BCIStream::InitialRedirection()
{
    return true;
}

static void Handle(const std::string &inMessage, const std::string &inTitle)
{
    std::string message = inMessage;
    if (message.find_last_of('\n') == message.length() - 1)
        message = message.substr(0, message.length() - 1);
    std::string title = FileUtils::ApplicationTitle();
    if (!inTitle.empty())
        title += " " + inTitle;
#if _WIN32
    ::MessageBoxA(nullptr, message.c_str(), title.c_str(), MB_OK);
#else
    std::cerr << title << ": " << message << std::endl;
#endif
}

bool BCIStream::CompressMessages()
{
    return false;
}

void BCIStream::PlainMessage(const std::string &s)
{
    Handle(s, "");
}

void BCIStream::DebugMessage(const std::string &s)
{
    Handle(s, "Debug Message");
}

void BCIStream::NoteMessage(const std::string& s)
{
    Handle(s, "Note");
}

void BCIStream::Warning(const std::string &s)
{
    Handle(s, "Warning");
}

void BCIStream::ConfigurationError(const std::string &s)
{
    Handle(s, "Error");
}

void BCIStream::RuntimeError(const std::string &s)
{
    Handle(s, "Error");
}

void BCIStream::LogicError(const std::string &s)
{
    Handle(s, "Error");
}
