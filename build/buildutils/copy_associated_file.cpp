////////////////////////////////////////////////////////////////////
// $Id$
// Author:      mellinger@neurotechcenter.org
// Description: Determine if a file exists.
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
////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include "FileUtils.h"
#include "StringUtils.h"

std::string usage = "Usage: copy_associated_file <main_file_extension> <associated_file_extension> <file> <target directory>";

int main(int argc, char *argv[])
{
    if (argc != 5) {
        std::cerr << usage << std::endl;
        return -1;
    }
    std::string mainExtension = argv[1], assocExtension = argv[2],
        file = argv[3], targetDir = argv[4];
    if (!FileUtils::IsFile(file)) {
        return -1;
    }
    std::string fileExtension = FileUtils::ExtractExtension(file);
    if (StringUtils::ToLower(fileExtension) != StringUtils::ToLower(mainExtension)) {
        return 0;
    }
    std::string assocFile = FileUtils::StripExtension(file) + assocExtension;
    if (!FileUtils::IsFile(assocFile)) {
        return 0;
    }
    if (!FileUtils::Copy(assocFile, targetDir + "/" + FileUtils::ExtractFile(assocFile))) {
        return -1;
    }
    return 0;
}
