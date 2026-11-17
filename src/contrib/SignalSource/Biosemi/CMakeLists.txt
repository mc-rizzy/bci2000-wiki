###########################################################################
## $Id: CMakeLists.txt 8584 2025-02-06 16:00:18Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for Biosemi2 source module

# Set the executable name
SET( EXECUTABLE_NAME Biosemi2 )

# Set the project specific sources
SET( FILES_PROJECT
  Biosemi2ADC.cpp
  Biosemi2Client.cpp
  lib/Labview_DLL.imports.cpp
)
INCLUDE_DIRECTORIES( lib )

BCI2000_INCLUDE( SOURCEFILTER )

IF(PROJECT_TARGET_PROCESSOR STREQUAL amd64)
SET( FILES_PROJECT
    ${FILES_PROJECT}
    lib/bin/Labview_DLL64.dll )
ELSEIF(PROJECT_TARGET_PROCESSOR STREQUAL x86)
SET( FILES_PROJECT
    ${FILES_PROJECT}
    lib/bin/Labview_DLL32.dll )
ENDIF()

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${FILES_PROJECT}" 
)
