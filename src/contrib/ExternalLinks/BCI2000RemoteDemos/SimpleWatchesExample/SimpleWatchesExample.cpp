////////////////////////////////////////////////////////////////////////////////
// $Id: SimpleWatchesExample.cpp 9224 2026-01-14 23:01:07Z mellinger $
// Author: mellinger@neurotechcenter.org
// Description: An example that illustrates how to watch expressions
//   (state value, signal) through BCI2000Remote.
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
#include "BCI2000Remote.h"

#include <iostream>
#include <string>

static void STDCALL OnWatchTriggered(void* data, const char* value)
{
    const std::string* pContext = static_cast<std::string*>(data);
    if (pContext)
        std::cout << *pContext << '\t';
    std::cout << value << std::endl;
}

int main(int argc, char* argv[])
{
    BCI2000Remote bci;
    bci.OperatorPath(""); // don't start BCI2000
    bci.Connect();
    if (!bci.Connected()) {
        std::cerr << "Could not connect to Operator at " << bci.TelnetAddress() << ".\n"
            << "Make sure BCI2000 is running before starting this program." << std::endl;
        return -1;
    }

    BCI2000Remote::Watch* pWatch = nullptr;
    std::string line, context;
    // Enter an empty line to abort processing the current expression.
    // Enter a new expression to continue.
    while (std::getline(std::cin, line)) {
        delete pWatch;
        if (line.empty()) {
            pWatch = nullptr;
        }
        else {
            context = line;
            pWatch = bci.NewWatch(line, &OnWatchTriggered, &context);
            if (!pWatch) {
                std::cerr << "Error creating watch" << std::endl;
            }
        }
    }
    delete pWatch;
    return 0;
}
