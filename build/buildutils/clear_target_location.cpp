////////////////////////////////////////////////////////////////////////////////
// $id$
// Author: mellinger@neurotechcenter.org
// Description: A utility that creates a Windows shortcut.
////////////////////////////////////////////////////////////////////////////////
#if _WIN32
#include <Windows.h>
#include "LCRandomGenerator.h"
#endif

#include "FileUtils.h"
#include "FilePath.h"
#include "SysError.h"
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: clear_target_location <existing path> <target directory>\n";
    }
    std::string name = FileUtils::ExtractName(argv[1]), targetDir = FileUtils::EnsureSeparator(argv[2]);
    std::string target = targetDir + name;
    if (!FileUtils::Exists(target))
        return 0;
    if (FileUtils::RemoveFile(target))
        return 0;
#if _WIN32 // cannot remove a file while it is open in Windows
    LCRandomGenerator rng;
    std::string tempDir = FileUtils::TemporaryDirectory();
    targetDir = FileUtils::AbsolutePath(targetDir);
    FilePath tempDirPath(tempDir), targetDirPath(targetDir);
    // Cannot move a file in use across drives (volumes)
    std::string movetoDir;
    if (tempDirPath.RootIsDrive() && targetDirPath.RootIsDrive() && tempDirPath.Root() == targetDirPath.Root()) {
        movetoDir = tempDir;
    }
    else {
        movetoDir = targetDir;
    }

    std::string moveto;
    do {
        moveto = movetoDir + rng.RandomName(16) + ".tmp";
    } while (FileUtils::Exists(moveto));
    if (!::MoveFileA(target.c_str(), moveto.c_str())) {
        std::cerr << "clear_target_location, MoveFile(): " << SysError().Message() << "\n";
        return -1;
    }
#if 0 // MoveFileEx(MOVEFILE_DELAY_UNTIL_REBOOT) requires admin privileges, 
      // so we accept some minor waste of space in the user's temp dir
    if (!::MoveFileExA(moveto.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT)) {
        std::cerr << "clear_target_location, MoveFileEx(): " << SysError().Message() << "\n";
        return -1;
    }
#endif
    return 0;
#else
    return -1;
#endif
}
