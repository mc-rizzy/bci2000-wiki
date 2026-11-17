###########################################################################
## $Id: IncludeExtension.cmake 8584 2025-02-06 16:00:18Z mellinger $
## Authors: kaleb.goering@gmail.com

IF(NOT WIN32)
  MESSAGE("**** EyetrackerLoggerTobii3 failed: Not supported on this platform.")
  RETURN()
ENDIF()

IF(PROJECT_TARGET_PROCESSOR STREQUAL amd64)
  SET(dll tetio64.dll)
ELSEIF(PROJECT_TARGET_PROCESSOR STREQUAL x86)
  SET(dll tetio32.dll)
ELSE()
  MESSAGE("**** EyetrackerLoggerTobii3 failed: Not supported on this architecture.")
  RETURN()
ENDIF()

SET( BCI2000_SIGSRC_FILES
   ${BCI2000_SIGSRC_FILES}
   ${BCI2000_EXTENSION_DIR}/TobiiNamespace.h
   ${BCI2000_EXTENSION_DIR}/EyetrackerLoggerTobii3.cpp
   ${BCI2000_EXTENSION_DIR}/extlib/include/core.imports.cpp
   ${BCI2000_EXTENSION_DIR}/extlib/dylib/${dll}
)

