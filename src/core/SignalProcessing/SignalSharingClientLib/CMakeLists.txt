###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org
## Description: Build information for the SignalSharingClientLib

set(NAME SignalSharingClientLib)

set(SRC_PROJECT
    SignalSharingClientLib.cpp
    ${PROJECT_SRC_DIR}/shared/bcistream/BCIStream_dll.cpp
)

if (WIN32)
  list(APPEND SRC_PROJECT
    DllMain.cpp
  )
endif()

utils_include(frameworks/Core)
utils_include(frameworks/LibTiny)
bci2000_use_inventory(SRC_PROJECT)
utils_set_appicon(puzzlepiece)
bci2000_add_target(
    INFO "SignalSharing client library" DLL 
    ${NAME}
    ${SRC_PROJECT}
)

# Add flag to declare exports when building library
set_property(TARGET ${NAME} APPEND PROPERTY COMPILE_FLAGS "-DSIGNAL_SHARING_CLIENT_LIBRARY")
