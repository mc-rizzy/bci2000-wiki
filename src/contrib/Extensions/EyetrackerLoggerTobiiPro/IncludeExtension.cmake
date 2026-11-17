###########################################################################
## $Id: IncludeExtension.cmake 9243 2026-01-28 17:40:46Z mellinger $
## Authors: kaleb.goering@gmail.com, mellinger@neurotechcenter.com

if(WIN32)

# Allowed versions: 1.8, 2.1
# After changing the version, in order for the changes to take effect, rebuild
# all source  modules.
set(TOBII_PRO_VERSION 2.1)
add_compile_definitions(TOBII_PRO_VERSION=${TOBII_PRO_VERSION})

set(dllfile)

if (PROJECT_TARGET_PROCESSOR STREQUAL amd64)
    set(dllfile
        ${BCI2000_EXTENSION_DIR}/extlib/v${TOBII_PRO_VERSION}/dylib/x64/tobii_research.dll 
    )
elseif (PROJECT_TARGET_PROCESSOR STREQUAL x86)
    set(dllfile
        ${BCI2000_EXTENSION_DIR}/extlib/v${TOBII_PRO_VERSION}/dylib/x32/tobii_research.dll 
    )
endif ()

if (NOT EXISTS "${dllfile}")
  message("EyetrackerLoggerTobiiPro: Unsupported architecture")
  return()
endif ()

set(BCI2000_SIGSRC_FILES
   ${BCI2000_SIGSRC_FILES}
   ${BCI2000_EXTENSION_DIR}/EyetrackerLoggerTobiiPro.cpp
   ${BCI2000_EXTENSION_DIR}/extlib/v${TOBII_PRO_VERSION}/include/tobii_research.imports.cpp
   ${BCI2000_EXTENSION_DIR}/VideoRecorder.cpp
   ${BCI2000_EXTENSION_DIR}/FrameComposer.cpp
   ${PROJECT_SRC_DIR}/shared/utils/Win32/Window.cpp
   ${PROJECT_SRC_DIR}/shared/utils/Win32/EventLoop.cpp
   ${PROJECT_SRC_DIR}/shared/utils/IniFile.cpp
   ${PROJECT_SRC_DIR}/extlib/com/ComStrings.cpp
   ${PROJECT_SRC_DIR}/extlib/com/ComPtr.h
   ${dllfile}
)

list(APPEND BCI2000_SIGSRC_LIBS 
    mf mfplat mfreadwrite mfuuid strmiids
)

else()
  message("EyetrackerLoggerTobiiPro: Only available on Windows")
endif()
