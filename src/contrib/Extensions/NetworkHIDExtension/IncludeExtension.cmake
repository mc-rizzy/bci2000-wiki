###########################################################################
## $Id: IncludeExtension.cmake 8584 2025-02-06 16:00:18Z mellinger $
## Authors: jezhill@gmail.com, griffin.milsap@gmail.com

IF( NOT WIN32 )
  MESSAGE( "**** NetworkHIDLogger may lack functionality on non-Windows platforms" )
ENDIF()

SET( BCI2000_SIGSRCINCDIRS
   ${BCI2000_SIGSRCINCDIRS}
   ${BCI2000_EXTENSION_DIR}
)

SET( BCI2000_SIGSRC_FILES
   ${BCI2000_SIGSRC_FILES}
   ${PROJECT_SRC_DIR}/extlib/NetworkHID/NetworkHID.cpp
   ${BCI2000_EXTENSION_DIR}/NetworkHIDExtension.cpp
)

