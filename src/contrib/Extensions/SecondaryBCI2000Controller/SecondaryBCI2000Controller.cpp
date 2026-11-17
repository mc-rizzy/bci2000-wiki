/////////////////////////////////////////////////////////////////////////////
// $Id: SecondaryBCI2000Controller.cpp 9502 2026-07-30 11:21:57Z mellinger $
// Author: mellinger@neurotechcenter.org
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
/////////////////////////////////////////////////////////////////////////////
#include "SecondaryBCI2000Controller.h"
#include "FileUtils.h"
#include "RunManager.h"
#include <thread>

Extension(SecondaryBCI2000Controller);

SecondaryBCI2000Controller::SecondaryBCI2000Controller()
    : mEnabled(false), mFilePart(0)
{
    PublishEnabler("SecondaryBCI2000");
}

SecondaryBCI2000Controller::~SecondaryBCI2000Controller()
{
}

void SecondaryBCI2000Controller::Publish()
{
    mEnabled = (OptionalParameter("SecondaryBCI2000", 0) != 0);
    if (!mEnabled)
        return;

    BEGIN_PARAMETER_DEFINITIONS
        "Source:Secondary%20BCI2000 int /SecondaryBCI2000= 1 0 0 1"
        " // Launch and control secondary BCI2000 instances (boolean)",

        "Source:Secondary%20BCI2000 matrix SecondaryBCI2000Instances= "
        " 1 { Telnet%20Port Startup%20Script } "
        "     4009          % "
        " // Telnet port for communication with Secondary instance, "
        "    Script file to start up Secondary instance",

        END_PARAMETER_DEFINITIONS
}

void SecondaryBCI2000Controller::AutoConfig()
{
    if (!mEnabled)
        return;

    mConnections.clear();
    std::string operatorPath = FileUtils::ParentDirectory(FileUtils::ExecutablePath()) + "Operator";
    auto SecondaryBCI2000Instances = Parameter("SecondaryBCI2000Instances");
    for (int i = 0; i < SecondaryBCI2000Instances->NumRows(); ++i) {
        auto pConnection = new BCI2000Remote;
        pConnection->OperatorPath(operatorPath);
        std::string prefix = "Secondary" + ((i == 0) ? "" : std::to_string(i + 1));
        pConnection->OperatorOptions("--AllowMultipleInstances --InstancePrefix " + prefix);

        std::string port, script;
        if (SecondaryBCI2000Instances->ColumnLabels().Exists("Telnet Port")) {
            port = SecondaryBCI2000Instances(i, "Telnet Port").ToString();
        }
        else {
            port = SecondaryBCI2000Instances(i, 0).ToString();
        }

        if (SecondaryBCI2000Instances->ColumnLabels().Exists("Startup Script")) {
            script = SecondaryBCI2000Instances(i, "Startup Script").ToString();
        }
        else {
            script = SecondaryBCI2000Instances(i, 1).ToString();
        }

        pConnection->TelnetAddress("localhost:" + port);
        pConnection->WindowVisible(false);
        pConnection->Connect();
        if (!pConnection->Connected()) {
            bcierr << "Could not start up or connect to secondary BCI2000 instance " << i + 1 << ": " << pConnection->Result();
        }
        else {
            std::string command = "EXECUTE SCRIPT \"" + script + "\"";
            int result = 0;
            pConnection->Execute(command, &result);
            if (result != 0)
            {
                bcierr << "Error when executing startup script " << i + 1 << ": " << pConnection->Result();
                pConnection->Disconnect();
            }
        }
        mConnections.emplace_back(pConnection);
    }
}

void SecondaryBCI2000Controller::Preflight() const
{
    if (!mEnabled)
        return;


    // Make sure current run is updated before SetConfig() is called in secondary instances.
    RunManager()->CurrentRun();

    for (int i = 0; i < mConnections.size(); ++i) {
        if (!mConnections[i]->Connected())
            continue;
        // Sync basic parameters between instances.
        for (auto p : { "SubjectName", "SubjectSession", "SubjectRun", "DataDirectory", "DataFile", "DataFileShm" })
        {
            std::string value = Parameter(p);
            mConnections[i]->SetParameter(p, value);
        }

        // Make sure the secondary system does not do file splitting by itself. Instead, we will send
        // updates to the FilePart event, which in turn will cause the next file part to begin.
        mConnections[i]->SetParameter("FileSplittingCondition", "");

        if (!mConnections[i]->SetConfig())
            bcierr << "Could not set configuration on secondary instance " << i + 1 << ": " << mConnections[i]->Result();
    }

    State("FilePart");
}

void SecondaryBCI2000Controller::Initialize()
{
    if (!mEnabled)
        return;
}

void SecondaryBCI2000Controller::StartRun()
{
    if (!mEnabled)
        return;

    // Make sure current run is updated before Start() is called in secondary instances.
    RunManager()->CurrentRun();

    mFilePart = 1;

    for (int i = 0; i < mConnections.size(); ++i) {
        if (!mConnections[i]->Connected())
            continue;
        if (!mConnections[i]->Start())
            bcierr << "Could not start run on secondary instance" << i + 1 << ": " << mConnections[i]->Result();
    }
}

void SecondaryBCI2000Controller::StopRun()
{
    if (!mEnabled)
        return;

    mFilePart = 0;

    for (int i = 0; i < mConnections.size(); ++i) {
        if (!mConnections[i]->Connected())
            continue;
        if (!mConnections[i]->Stop())
            bcierr << "Could not stop run on secondary instance" << i + 1 << ": " << mConnections[i]->Result();
    }
}

void SecondaryBCI2000Controller::Process()
{
    if (!mEnabled || mFilePart < 1)
        return;

    int nextFilePart = State("FilePart")(Statevector->Samples() - 1);
    if (nextFilePart > mFilePart)
    {
        mFilePart = nextFilePart;
        for (int i = 0; i < mConnections.size(); ++i) {
            if (!mConnections[i]->Connected())
                continue;
            if (!mConnections[i]->SetEventVariable("FilePart", nextFilePart))
                bcierr << "Could not set file part on secondary instance" << i + 1 << ": " << mConnections[i]->Result();
        }
    }
}

void SecondaryBCI2000Controller::Halt()
{
    if (!mEnabled)
        return;

    for (auto& pConnection : mConnections) {
        if (!pConnection->Connected())
            continue;
        pConnection->Stop();
        pConnection->Disconnect();
    }
    mConnections.clear();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
