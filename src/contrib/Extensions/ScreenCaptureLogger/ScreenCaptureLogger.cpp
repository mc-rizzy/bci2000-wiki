/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: mellinger@neurotechcenter.org
// Description: The screen capture logger logs screen content, and writes
//   it into one or more video files. Also, it logs the current frame
//   number into an event state.
//
// Parameterization:
//   Screen capture logging is enabled from the command line adding
//     --LogScreenCapture=1
//
// Event Variables:
//   ScreenFrame<n> - The current frame number for capture index n 
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
#include "ScreenCaptureLogger.h"
#include "FileUtils.h"
#include "RunManager.h"

#define NUM_OF_SCREEN_CAPTURE_EVENTS 4

Extension(ScreenCaptureLogger);

static void IndexTooLargeInfo()
{
    bcierr << "The max webcam index supported by this version of BCI2000 is " << NUM_OF_SCREEN_CAPTURE_EVENTS
        << ". To record from more screen areas, change NUM_OF_SCREEN_CAPTURE_EVENTS macro in ScreenCaptureLogger.cpp to the "
        << "max index or number of webcams you want to record from and recompile.";
}

ScreenCaptureLogger::ScreenCaptureLogger()
{
    PublishEnabler("LogScreenCapture");
}

ScreenCaptureLogger::~ScreenCaptureLogger()
{
    Halt();
}

void ScreenCaptureLogger::Publish()
{
    if(!OptionalParameter("LogScreenCapture"))
        return;

    BEGIN_PARAMETER_DEFINITIONS
        "Source:ScreenCapture int /LogScreenCapture= 1 0 0 1"
        " // Allow logging screen capture (boolean)",

        "Source:ScreenCapture matrix ScreenCaptureAreas= 1 { left top width height spatial%20decimation fps encoder%20profile compression }"
        " 0 0 720 480 1 15Hz 2 100 % %"
        " // Desired location and fps for each capture area",

        "Source:ScreenCapture int DisplayCaptureStream= 1 1 0 1"
        " // Display each screen capture in a window (boolean)",

        END_PARAMETER_DEFINITIONS

        for (int i = 0; i < NUM_OF_SCREEN_CAPTURE_EVENTS; ++i)
        {
            std::ostringstream definition;
            definition << "ScreenCaptureFrame" << i + 1 << " 32 0 0 0";
            BEGIN_EVENT_DEFINITIONS
                definition.str(),
            END_EVENT_DEFINITIONS
        }
}

void ScreenCaptureLogger::Preflight() const
{
    if (!OptionalParameter("LogScreenCapture"))
        return;

    auto screenCaptureAreas = Parameter("ScreenCaptureAreas");
    for (int row = 0; row < screenCaptureAreas->NumRows(); ++row) {
        auto pCapture = CreateScreenCapture(screenCaptureAreas, row);
        if (!pCapture->Error().empty())
            bcierr << pCapture->Error();
    }

    State("FilePart");
}

void ScreenCaptureLogger::Initialize()
{
    if (!OptionalParameter("LogScreenCapture"))
        return;

    mScreenCaptures.clear();
    auto screenCaptureAreas = Parameter("ScreenCaptureAreas");
    for (int row = 0; row < screenCaptureAreas->NumRows(); ++row) {
        auto pCapture = CreateScreenCapture(screenCaptureAreas, row);
        pCapture->StartupCaptureStream();
        if (!pCapture->Error().empty())
            bcierr << pCapture->Error();
        mScreenCaptures.push_back(std::move(pCapture));
    }

    for (auto& p : mScreenCaptures)
    {
        if (Parameter("DisplayCaptureStream"))
            p->ShowWindow();
        if (!p->Error().empty())
            bcierr << p->Error();
    }
}

void ScreenCaptureLogger::StartRun()
{
    mFilePart = 1;
    std::string baseName = FileUtils::StripExtension(RunManager()->CurrentRun());
    for (const auto& p : mScreenCaptures) {
        if (!p->IsDefunct()) {
            p->StartRecording(baseName);
            if (!p->Error().empty())
                bcierr << p->Error();
        }
    }
}

void ScreenCaptureLogger::StopRun()
{
    for (const auto& p : mScreenCaptures) {
        if (!p->IsDefunct()) {
            p->StopRecording();
            if (!p->Error().empty())
                bcierr << p->Error();
        }
    }
}

void ScreenCaptureLogger::Process()
{
    if (!OptionalParameter("LogScreenCapture"))
        return;

    int nextFilePart = State("FilePart")(Statevector->Samples() - 1);
    if (nextFilePart > mFilePart && nextFilePart > 1)
    {
        mFilePart = nextFilePart;
        for (const auto& p : mScreenCaptures) {
            if (!p->IsDefunct()) {
                p->SwitchToFilePart(mFilePart);
                if (!p->Error().empty())
                    bcierr << p->Error();
            }
        }
    }
}

void ScreenCaptureLogger::Halt()
{
    mScreenCaptures.clear();
}

std::unique_ptr<ScreenCaptureThread> ScreenCaptureLogger::CreateScreenCapture(const ParamRef& areas, int row)
{
    int left = areas(row, "left");
    int top = areas(row, "top");
    int width = areas(row, "width");
    int height = areas(row, "height");
    float decimation = areas(row, "spatial decimation");
    float fps = areas(row, "fps").InHertz();
    int profile = areas(row, "encoder profile");
    float compression = areas(row, "compression");
    if (compression < 1)
        compression = 1;
    auto pScreenCapture = std::make_unique<ScreenCaptureThread>(row + 1);
    pScreenCapture->Initialize(left, top, width, height, decimation, fps, profile, compression);
    return pScreenCapture;
}
