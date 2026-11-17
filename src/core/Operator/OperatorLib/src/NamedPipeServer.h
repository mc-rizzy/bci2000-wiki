////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: A named pipe server that runs inside a BCI2000
//   operator module. Named pipes are more secure than sockets because they
//   allow to control access by user.
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
#ifndef NAMED_PIPE_SERVER_H
#define NAMED_PIPE_SERVER_H

#include "Sockets.h"
#include "Thread.h"

class NamedPipeServer : private Thread
{
  public:
    NamedPipeServer(class StateMachine &, const std::string &name);
    ~NamedPipeServer();
    std::string Name() const;

  protected:
    int OnExecute();

  private:
    class Session;
    std::string mName;
    ServerUnixSocket mListeningSocket;
    class StateMachine &mrStateMachine;
};

#endif // NAMED_PIPE_SERVER_H
