###########################################################################
## $Id: IncludeExtension.cmake 8584 2025-02-06 16:00:18Z mellinger $
## Authors: kaleb.goering@gmail.com

IF( NOT WIN32 )
  MESSAGE( "**** EyetrackerLoggerTobiiX failed: Not supported on this platform" )
  RETURN()
ENDIF()

IF(PROJECT_TARGET_PROCESSOR STREQUAL amd64)
  SET(dll Tobii.EyeX.Client64.dll)
ELSEIF(PROJECT_TARGET_PROCESSOR STREQUAL x86)
  SET(dll Tobii.EyeX.Client32.dll)
ELSE()
  MESSAGE("**** EyetrackerLoggerTobiiX failed: Not supported on this architecture.")
  RETURN()
ENDIF()

SET( BCI2000_SIGSRC_FILES
   ${BCI2000_SIGSRC_FILES}
   ${BCI2000_EXTENSION_DIR}/EyetrackerLoggerTobiiX.cpp
   ${BCI2000_EXTENSION_DIR}/extlib/include/eyex/EyeX.imports.cpp
   ${BCI2000_EXTENSION_DIR}/extlib/dylib/${dll}
)
