###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org

list(APPEND BCI2000_SIGSRC_INCLUDE_EXTLIB
  FFMPEG
)

list( APPEND BCI2000_SIGSRC_FILES
   ${BCI2000_EXTENSION_DIR}/ScreenCaptureLogger.cpp
   ${BCI2000_EXTENSION_DIR}/ScreenCaptureThread.cpp
   ${BCI2000_EXTENSION_DIR}/Capture.h
   ${PROJECT_SRC_DIR}/shared/utils/AV/AVEncoder.cpp
   ${PROJECT_SRC_DIR}/shared/utils/AV/AVError.cpp
   ${PROJECT_SRC_DIR}/shared/utils/AV/AVLog.cpp
)

if (WIN32)
   list(APPEND BCI2000_SIGSRC_FILES
      ${BCI2000_EXTENSION_DIR}/Capture_Win32.cpp
   )
   list(APPEND BCI2000_SIGSRC_LIBS
      mfplat.lib mf.lib mfreadwrite.lib mfuuid.lib strmiids.lib propsys.lib
   )
elseif (APPLE)
   list(APPEND BCI2000_SIGSRC_FILES
     ${BCI2000_EXTENSION_DIR}/Capture_macOS.mm
  )
  list(APPEND BCI2000_SIGSRC_LIBS
    "-framework Foundation"
    "-framework CoreMedia"
    "-framework AVFoundation"
    "-framework ScreenCaptureKit"
  )
else()
   list(APPEND BCI2000_SIGSRC_FILES
     ${BCI2000_EXTENSION_DIR}/Capture_Linux.cpp
  )
  list(APPEND BCI2000_SIGSRC_LIBS
    xcb xcb-shm
  )
endif()

