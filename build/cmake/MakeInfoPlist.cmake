###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org
## Description: CMake script to automate creation of an Info.plist file
##   for macOS bundles.

string(RANDOM RAND_ID)
configure_file("${CMAKE_CURRENT_LIST_DIR}/../buildutils/Info.plist.in" ${OUT_DIR}/Info.plist)

