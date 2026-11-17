###########################################################################
## $Id: CMakeLists.txt 8584 2025-02-06 16:00:18Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for vAmpSource

set(FirstAmpDll)
set(MSVRCDLL)
if( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
  set( FirstAmpDll lib/bin/FirstAmp64.dll )
  set( MSVRCDLL lib/msvrc64/msvcr100.dll )
elseif( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
  set( FirstAmpDll lib/bin/FirstAmp32.dll )
  set( MSVRCDLL lib/msvrc32/msvcr100.dll )
endif()

bci2000_include( Math )
bci2000_use( Qt )
bci2000_add_signal_source_module( vAmpSource
  vAmpADC.cpp 
  vAmpChannelInfo.cpp
  vAmpThread.cpp
  vAmpDisplay.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Qt/QtMain.cpp
  lib/FirstAmp.imports.cpp
  ${BCI2000_SRC_DIR}/shared/gui/GUI.h
  ${FirstAmpDll}
  ${MSVRCDLL}
)
