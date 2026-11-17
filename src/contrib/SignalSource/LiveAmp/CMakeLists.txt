###########################################################################
## Authors:
## Description: Build information for the LiveAmp source module
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME LiveAmpSource )

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Set the project specific sources
SET( SRC_PROJECT
  LiveAmpADC.cpp
  lib/LiveAmpLib2.imports.cpp
)

IF( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
  list( APPEND SRC_PROJECT lib/dylib/LiveAmpLib2_64.dll )
ELSEIF( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
  list( APPEND SRC_PROJECT lib/dylib/LiveAmpLib2_32.dll )
ENDIF()

BCI2000_INCLUDE( "SOURCEFILTER" )

BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${SRC_PROJECT}" "${HDR_PROJECT}" 
)
