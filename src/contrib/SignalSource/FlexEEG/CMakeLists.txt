###########################################################################
## Authors: t.moore
## Description: Build information for the NC_flexX source module
###########################################################################

# Set the executable name
SET( EXECUTABLE_NAME FlexEEG )

# Set the project specific sources
SET( FILES_PROJECT
  NC_flexXADC.cpp
  lib/flexX.imports.cpp
)
INCLUDE_DIRECTORIES( lib )

IF(PROJECT_TARGET_PROCESSOR STREQUAL amd64)
  SET( FILES_PROJECT
    ${FILES_PROJECT}
    lib/flexX64.dll
  )
ELSEIF(PROJECT_TARGET_PROCESSOR STREQUAL x86)
  SET( FILES_PROJECT
    ${FILES_PROJECT}
    lib/flexX32.dll
  )
ENDIF()

# Create the signal source module
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  "${EXECUTABLE_NAME}" 
  "${FILES_PROJECT}" 
  SUPPORT batch/*=>/batch
  SUPPORT parms/*=>/parms/fragments/amplifiers
)
