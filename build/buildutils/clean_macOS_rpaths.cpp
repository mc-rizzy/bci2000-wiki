#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "ProcessUtils.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Missing argument: expected binary file"
                  << std::endl;
        return -1;
    }
    
    std::string binaryFile = argv[1];
    std::ostringstream oss;
    int result = 0;
    if (!ProcessUtils::ExecuteSynchronously("otool", "-l \"" + binaryFile + "\"", oss, result)) {
        return -1;
    }
    if (result != 0) {
        return result;
    }
    std::vector<std::string> rpaths, duplicates;
    std::istringstream iss(oss.str());
    std::string line;
    while (std::getline(iss, line)) {
        size_t pos = line.find(" path ");
        if (pos != std::string::npos) {
            line = line.substr(pos + 6);
            pos = line.find(" ");
            line = line.substr(0, pos);
            if (std::find(rpaths.begin(), rpaths.end(), line) != rpaths.end()) {
                duplicates.push_back(line);
            }
            else {
                rpaths.push_back(line);
            }
        }
    }
    oss.str("");
    oss.clear();
    for (const auto& duplicate : duplicates) {
        if (!ProcessUtils::ExecuteSynchronously("install_name_tool", "-delete_rpath " + duplicate + " \"" + binaryFile + "\"", oss, result)) {
            return -1;
        }
        if (result != 0) {
            return result;
        }
        
    }
    return 0;
}
