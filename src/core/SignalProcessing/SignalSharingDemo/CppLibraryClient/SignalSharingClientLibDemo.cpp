////////////////////////////////////////////////////////////////////////////////
// $Id$
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
#include "SignalSharingClientLib.h"

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

static std::string SSC_ResultToString(SSC_RESULT result)
{
    switch (result) {
    case SSC_SUCCESS:
        return "Success";
    case SSC_INVALID_ARG:
        return "Invalid argument";
    case SSC_CANNOT_LISTEN:
        return "Cannot listen on given address";
    case SSC_CANNOT_FULFILL_REQUEST:
        return "Cannot fulfill request at this time";
    }
    return "Unknown error";
}

// Translate an error code into an exception
struct SSC_SucceedOrThrow
{
    SSC_SucceedOrThrow(SSC_RESULT result)
    {
        if (result != SSC_SUCCESS) {
            throw std::runtime_error(SSC_ResultToString(result));
        }
    }
};

// The data we need inside the callback functions
struct ConnectorData
{
    SSC_HANDLE h = nullptr;
    std::ofstream paramfile, datafile;
    std::mutex mutex;
};

// Callback function for handling parameters
static void HandleParameters(void* pData)
{
    std::cout << "Received parameters ..." << std::endl;
    auto pConnectorData = static_cast<ConnectorData*>(pData);
    const char* const* parameterLines = nullptr;
    int count = 0;
    if (SSC_SUCCESS == ::SSC_LockParameters(pConnectorData->h, &parameterLines, &count)) {
        std::unique_lock lock(pConnectorData->mutex);
        for (int i = 0; i < count; ++i) {
            pConnectorData->paramfile << parameterLines[i] << "\n";
        }
        lock.unlock();
        ::SSC_ReleaseParameters(pConnectorData->h);
    }
}

// Callback function for handling signals
static void HandleSignal(void* pData)
{
    std::cout << "Received signal ..." << std::endl;
    auto pConnectorData = static_cast<ConnectorData*>(pData);
    union { const double* d; const char* c; } pSignal = { nullptr };
    int channels = 0, samples = 0;
    if (SSC_SUCCESS == ::SSC_LockSignal(pConnectorData->h, &pSignal.d, &channels, &samples, nullptr)) {
        std::unique_lock lock(pConnectorData->mutex);
        // Transpose data so we have a continuous stream of samples across channels
        for (int sample = 0; sample < samples; ++sample) {
            for (int ch = 0; ch < channels; ++ch) {
                // Data comes in channel-major format
                int idx = ch * samples + sample;
                pConnectorData->datafile.write(pSignal.c + idx * sizeof(double), sizeof(double));
            }
        }
        lock.unlock();
        ::SSC_ReleaseSignal(pConnectorData->h);
    }
}

int main(int argc, char **argv)
{
    std::string address = "localhost:1879";
    if (argc > 1)
        address = argv[1];

    std::string paramfile = "sscparams.prm";
    if (argc > 2)
        paramfile = argv[2];

    std::string datafile = "sscdata.raw";
    if (argc > 3)
        datafile = argv[3];

    // The main function's return value
    int result = 0;

    ConnectorData data;
    // Callback functions are called from a separate thread, so let's lock the mutex
    // (not really necessary here because the thread will first start inside SSC_CreateConnector() below)
    std::unique_lock lock(data.mutex);
    data.paramfile.open(paramfile);
    data.datafile.open(datafile, std::ios::binary | std::ios::out);
    lock.unlock();
    try {
        SSC_SucceedOrThrow result = ::SSC_CreateConnector(address.c_str(), &data.h);
        result = ::SSC_RegisterParametersCallback(data.h, &HandleParameters, &data);
        result = ::SSC_RegisterSignalCallback(data.h, &HandleSignal, &data);

        // Callback functions are called from a separate thread, so just wait for user input
        std::cout << "Press <Enter> to quit" << std::endl;
        std::string ignored;
        std::getline(std::cin, ignored);
    }
    catch (const std::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        result = -1;
    }
    ::SSC_DeleteConnector(data.h);
    return result;
}
