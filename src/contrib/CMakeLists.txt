###########################################################################
## $Id: CMakeLists.txt 9159 2025-11-03 15:13:12Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}Contrib/" )

ADD_SUBDIRECTORY( Extensions )
ADD_SUBDIRECTORY( SignalSource )
ADD_SUBDIRECTORY( SignalProcessing )
ADD_SUBDIRECTORY( Application )

utils_match_subdir(BUILD_MODULES Hyperscanning)
UTILS_MATCH_SUBDIR( BUILD_MODULES OperatorLibApplications )
UTILS_MATCH_SUBDIR( BUILD_MODULES ExternalLinks )
UTILS_MATCH_SUBDIR( BUILD_MODULES BCPy2000 )


