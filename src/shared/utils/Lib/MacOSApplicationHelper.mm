////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mellinger@neurotechcenter.org
// Description: Objective-C source file using NSWorkspace to launch a
//   child process.
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
#include "MacOSApplicationHelper.h"

#include <Cocoa/Cocoa.h>
#include <mutex>
#include <condition_variable>

namespace Tiny
{

namespace ProcessUtils
{

std::pair<pid_t, int>
MacOSApplicationHelper::StartApplicationBundle(const std::string& bundlePath, const std::vector<std::string>& args)
{
    __block pid_t pid = 0;
    __block int err = 0;
    dispatch_semaphore_t sem = ::dispatch_semaphore_create(0);

    @autoreleasepool {
        NSString* urlString = [NSString stringWithCString:bundlePath.c_str() encoding:NSUTF8StringEncoding];
        NSURL* url = [NSURL fileURLWithPath:urlString];
        NSWorkspace* ws = [NSWorkspace sharedWorkspace];
        NSWorkspaceOpenConfiguration* config = [NSWorkspaceOpenConfiguration configuration];
        NSMutableArray* argsArray = [[NSMutableArray alloc] init];
        for (const auto& arg : args) {
            [argsArray addObject:[NSString stringWithCString:arg.c_str() encoding:NSUTF8StringEncoding]];
        }
        config.arguments = argsArray;
        [ws openApplicationAtURL:url configuration:config completionHandler:^(NSRunningApplication* pApp, NSError* pErr) {
                   if (pApp) {
                       pid = pApp.processIdentifier;
                   }
                   if (pErr) {
                       err = pErr.code;
                   }
                   ::dispatch_semaphore_signal(sem);
        }];
    }

    ::dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
    ::dispatch_release(sem);
    return std::make_pair(pid, err);
}

} // namespace ProcessUtils

} // namespace Tiny

