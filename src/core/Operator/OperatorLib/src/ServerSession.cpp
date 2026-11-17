////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: A class that represents a client session for a telnet or
//   named pipe connection to the operator module.
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
#include "ServerSession.h"

#include "VersionInfo.h"

ServerSession::ServerSession(::StateMachine& stateMachine, std::streambuf* pBuf)
    : Thread(true), ScriptInterpreter(stateMachine), mStream(pBuf)
{
    ScriptInterpreter::WriteLineHandler(&OnWriteLine, this);
    ScriptInterpreter::ReadLineHandler(&OnReadLine, this);
}

ServerSession::~ServerSession()
{
}

void ServerSession::Start()
{
    Thread::StartIfNotRunning();
}

void ServerSession::RequestTermination()
{
    ScriptInterpreter::Abort();
    Thread::Terminate();
}

int ServerSession::OnExecute()
{
    LocalVariables()[RemoteHostName()] = RemoteHost();
    WriteHello().WriteNewline().WritePrompt();
    bool ok = true;
    while (mStream && ok)
        ok = ReadCharacter();
    return 0;
}

void ServerSession::OnScriptError(const std::string &inMessage)
{
    Write(inMessage).WriteNewline();
    ScriptInterpreter::OnScriptError(inMessage);
}

bool ServerSession::OnWriteLine(void *inInstance, const std::string &inLine)
{
    ServerSession *this_ = reinterpret_cast<ServerSession *>(inInstance);
    this_->Write(inLine).WriteNewline();
    return !!(this_->mStream << std::flush);
}

bool ServerSession::OnReadLine(void *inInstance, std::string &outLine)
{
    ServerSession *this_ = reinterpret_cast<ServerSession *>(inInstance);
    this_->Write("\\AwaitingInput:").WriteNewline();
    this_->mStream << std::flush;
    bool result = !!std::getline(this_->mStream, outLine);
    if (result)
        this_->Write("\\AcknowledgedInput").WriteNewline();
    if (!outLine.empty() && *outLine.rbegin() == '\r')
        outLine = outLine.substr(0, outLine.length() - 1);
    return result;
}

bool ServerSession::ReadCharacter()
{
    while (mStream.rdbuf()->in_avail() || WaitForInput())
    {
        int c = mStream.get();
        if (!mStream)
            return false;
        switch (c)
        {
        case '\n':
            if (ScriptInterpreter::Execute(mLineBuffer))
            {
                const std::string &r = ScriptInterpreter::Result();
                Write(r);
                if (!r.empty() && *r.rbegin() != '\n')
                    WriteNewline();
            }
            if (ScriptInterpreter::Flags() & (CommandInterpreter::exit_ | CommandInterpreter::return_))
            {
                return false;
            }
            WritePrompt();
            mLineBuffer.clear();
            break;
        case '\r':
        case '\x07': // Bell
            break;
        case '\x08': // Backspace
            if (!mLineBuffer.empty())
                mLineBuffer = mLineBuffer.substr(0, mLineBuffer.length() - 1);
            break;
        case 255: // Interpret As Command
            if (mStream.rdbuf()->in_avail() || WaitForInput())
            {
                switch (mStream.get())
                {
                case 247: // Erase Character
                    if (!mLineBuffer.empty())
                        mLineBuffer = mLineBuffer.substr(0, mLineBuffer.length() - 1);
                    break;
                case 248: // Erase Line
                    mLineBuffer.clear();
                    break;
                default:
                    while (mStream && mStream.rdbuf()->in_avail() && mStream.peek() > 127)
                        mStream.get();
                }
                break;
            }
        default:
            mLineBuffer += c;
        }
    }
    return mStream.rdbuf()->in_avail() || WaitForInput();
}

ServerSession &ServerSession::WriteHello()
{
    mStream << "BCI2000 Version " << VersionInfo::Current[VersionInfo::VersionID];
    std::string hostname = Socket::Hostname();
    if (!hostname.empty())
        mStream << " on " << hostname;
    WriteNewline();
    mStream << "Type 'help' for a list of commands.";
    return *this;
}

ServerSession &ServerSession::Write(const std::string &inString)
{
    const char *p, *q;
    for (p = inString.c_str(), q = p; *p; ++p)
    {
        if (*p == '\n')
        {
            Assert(*q || q == p);
            mStream.write(q, p - q);
            WriteNewline();
            q = p + 1;
        }
    }
    Assert(*q || q == p);
    mStream.write(q, p - q);
    return *this;
}

ServerSession &ServerSession::WriteNewline()
{
    mStream.write("\r\n", 2);
    return *this;
}

ServerSession &ServerSession::WritePrompt()
{
    mStream.put('>').flush();
    return *this;
}

