###########################################################################
## $Id: CMakeLists.txt 8584 2025-02-06 16:00:18Z mellinger $
## Authors: Paul Ignatenko <paul dot ignatenko at gmail dot com>
## Description: This module builds the libraries and sources needed to use Brain 
## Vision's actiCHamp amplifier.
##              
##              
###########################################################################
IF( WIN32 )

# Set the executable name
SET( EXECUTABLE_NAME actiCHampSource )

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Set the project specific sources
SET( SRC_PROJECT
  actiCHampBufferedADC.cpp
  actiCHampDevice.cpp
  ChannelRouter.cpp
  lib/ActiChamp.imports.cpp
  COMPANION lib/dylib/ActiChamp.bit
  COMPANION lib/dylib/ActiChampX.bit
)

IF( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
  list( APPEND SRC_PROJECT lib/dylib/ActiChamp_x64.dll )
ELSEIF( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
  list( APPEND SRC_PROJECT lib/dylib/ActiChamp_x86.dll )
ENDIF()


BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)

ENDIF( WIN32 )
