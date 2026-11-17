###########################################################################
## Authors:
## Description: Build information for the actiCHampPlus source module
###########################################################################
IF( WIN32 )

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( MATH )

# Set the project specific sources
SET( SRC_PROJECT
  actiCHampPlusADC.cpp
  lib/AmplifierSDK.imports.cpp
  lib/AmplifierSDK.imports.h
  actiCHampPlusADC.h
  lib/AmplifierSDK.h
  lib/Amplifier_LIB.h
  lib/RawDataHandlerExample.h
  SDK.h
)


IF( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
  list( APPEND SRC_PROJECT lib/dylib/x64/AmplifierSDK.dll )
  list( APPEND SRC_PROJECT lib/dylib/x64/actiCHampLib.dll )
  list( APPEND SRC_PROJECT lib/dylib/x64/ActiCHamp_x64.dll)
ELSEIF( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
  list( APPEND SRC_PROJECT lib/dylib/x86/AmplifierSDK.dll )
  list( APPEND SRC_PROJECT lib/dylib/x86/actiCHampLib.dll )
  list( APPEND SRC_PROJECT lib/dylib/x86/ActiCHamp_x86.dll)
ENDIF()

bci2000_add_signal_source_module(
  actiCHampPlus
  "${SRC_PROJECT}"
)

ENDIF( WIN32 )
