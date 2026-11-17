#include <unistd.h>
#include <iostream>
#include "FileUtils.h"

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Missing arguments: expected source file "
                  << "and target link name"
                  << std::endl;
        return -1;
    }
    
    const char* pSourceFile = argv[1];
    const char* pTargetLink = argv[2];
    std::string targetDir = FileUtils::ParentDirectory(pTargetLink);
    std::string linkContent = FileUtils::RelativePath(pSourceFile, targetDir);
    if (::symlink(linkContent.c_str(), pTargetLink)) {
        std::cerr << ::strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}
