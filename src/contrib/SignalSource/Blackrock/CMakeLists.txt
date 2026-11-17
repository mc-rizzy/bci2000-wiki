###########################################################################
## $Id: CMakeLists.txt 8584 2025-02-06 16:00:18Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for the Blackrock source module
###########################################################################

IF( WIN32 )

if (PROJECT_TARGET_PROCESSOR STREQUAL amd64)
  set(lib ${PROJECT_SRC_DIR}/contrib/SignalSource/Blackrock/lib/x64/cbsdkx64.lib)
elseif (PROJECT_TARGET_PROCESSOR STREQUAL x86)
  set(lib ${PROJECT_SRC_DIR}/contrib/SignalSource/Blackrock/lib/x86/cbsdk.lib)
else ()
  return ()
endif ()

# Set the executable name
SET( EXECUTABLE_NAME Blackrock )

SET( SRC_PROJECT 
  BlackrockADC.cpp
)

SET( HDR_PROJECT
  BlackrockADC.h
)

SET( FILES_PROJECT
  ${FILES_PROJECT}
  SUPPORT batch/*=>/batch
  SUPPORT parms/*=>/parms/fragments/amplifiers
)

INCLUDE_DIRECTORIES( include )

# Add the source module to the Makefile or compiler project file:
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}"
  "${SRC_PROJECT}" "${HDR_PROJECT}"
  "${FILES_PROJECT}"
)

target_link_libraries( ${EXECUTABLE_NAME} ${lib} )

ENDIF( WIN32 )

