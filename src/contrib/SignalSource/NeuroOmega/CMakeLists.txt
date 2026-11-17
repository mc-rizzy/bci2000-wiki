###########################################################################
## $Id: CMakeLists.txt 8584 2025-02-06 16:00:18Z mellinger $
## Authors: Alexander Belsten (belsten@neurotechcenter.org)
## Description: Build information for the NeuroOmegaADC source module
###########################################################################

if(WIN32)

# Set the executable name
SET( EXECUTABLE_NAME NeuroOmegaADC)

# Set the project specific sources
SET( SRC_PROJECT
  NeuroOmegaADC.cpp
  include/NeuroOmega.imports.cpp
)

# specify dll files to be copied
IF(PROJECT_TARGET_PROCESSOR STREQUAL amd64)
  SET( FILES_PROJECT
    ${FILES_PROJECT}
    lib/x64/NeuroOmega_x64.dll
  )
ELSEIF(PROJECT_TARGET_PROCESSOR STREQUAL x86)
  SET( FILES_PROJECT
    ${FILES_PROJECT}
    lib/x86/NeuroOmega.dll
  )
ENDIF()


# Add the source module to the Makefile or compiler project file:
BCI2000_ADD_SIGNAL_SOURCE_MODULE(
  "${EXECUTABLE_NAME}"
  "${SRC_PROJECT}" "${FILES_PROJECT}"
)

endif()
