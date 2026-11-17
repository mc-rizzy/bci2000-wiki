###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including the wayland
##   screen capture lib

SET( EXTLIB_OK TRUE )
SET( HDR_EXTLIB ${PROJECT_SRC_DIR}/extlib/screencapture-wayland/libscreencapture-wayland/common.hpp )
SET( SRC_EXTLIB
    ${PROJECT_SRC_DIR}/extlib/screencapture-wayland/libscreencapture-wayland/common.cpp
    ${PROJECT_SRC_DIR}/extlib/screencapture-wayland/libscreencapture-wayland/PipeWireModule/PipeWireStream.cpp
    ${PROJECT_SRC_DIR}/extlib/screencapture-wayland/libscreencapture-wayland/PortalModule/xdg-desktop-portal.cpp
    
)
SET( INC_EXTLIB 
    ${PROJECT_SRC_DIR}/extlib/screencapture-wayland/libscreencapture-wayland
    ${PROJECT_SRC_DIR}/extlib/screencapture-wayland/libscreencapture-wayland/include
    /usr/include/pipewire-0.3
    /usr/include/spa-0.2
)
SET ( LIBS_EXTLIB sdbus-c++ pipewire-0.3)

