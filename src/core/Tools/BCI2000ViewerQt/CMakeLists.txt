###########################################################################
## $Id: CMakeLists.txt 9485 2026-07-21 17:16:08Z mellinger $
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for BCI2000ViewerQt

# Set the executable name
SET( EXECUTABLE_NAME BCI2000Viewer )

BCI2000_INCLUDE(SoundTouch)

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
  BCI2000Viewer.ui
  BCI2000Viewer.cpp
  SignalWidget.cpp
  ${BCI2000_SRC_DIR}/shared/gui/SignalDisplay.cpp
  ${BCI2000_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Qt/ExtendedColorSelector.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Qt/QtWidgetProxy.h
  PlaybackControl.cpp
  VideoWidget.cpp
  ${BCI2000_SRC_DIR}/shared/utils/AV/AVDecoder.cpp
  ${BCI2000_SRC_DIR}/shared/utils/AV/AVUtils.cpp
  ${BCI2000_SRC_DIR}/shared/utils/AV/AVPlayer.cpp
  ${BCI2000_SRC_DIR}/shared/utils/AV/AudioSink.cpp
  ${BCI2000_SRC_DIR}/shared/utils/AV/AudioDevice.cpp
  ${BCI2000_SRC_DIR}/shared/utils/AV/AudioPlayer.cpp
  ${BCI2000_SRC_DIR}/shared/utils/ClockTick.cpp
  NotesWidget.cpp
  TimeStretchAudioPlayer.cpp
)
if(APPLE)
  list(APPEND SRC_PROJECT
    Info.plist
  )
endif()

utils_add_resource(autoscale_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/autoscale_96.png)
utils_add_resource(zoom_in_h_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/zoom-in-h_96.png)
utils_add_resource(zoom_out_h_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/zoom-out-h_96.png)
utils_add_resource(zoom_reset_h_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/zoom-reset-h_96.png)
utils_add_resource(zoom_in_v_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/zoom-in-v_96.png)
utils_add_resource(zoom_out_v_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/zoom-out-v_96.png)
utils_add_resource(zoom_reset_v_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/zoom-reset-v_96.png)
utils_add_resource(plus_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/plus_96.png)
utils_add_resource(minus_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/minus_96.png)
utils_add_resource(grid_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/grid_96.png)
utils_add_resource(colorwheel_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/colorwheel_96.png)
utils_add_resource(light_dark_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/light-dark_96.png)

BCI2000_INCLUDE(math)
BCI2000_INCLUDE(ffmpeg)
BCI2000_INCLUDE(PortAudio)
BCI2000_ADD_TOOLS_GUIAPP( ${EXECUTABLE_NAME} ${SRC_PROJECT} )
