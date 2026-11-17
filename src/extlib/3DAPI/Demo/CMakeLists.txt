###########################################################################
## $Id: CMakeLists.txt 8664 2025-03-28 14:58:51Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for 3D API Demo in Qt

IF(BUILD_DEMOS)

SET(EXECUTABLE_NAME Demo3DAPI)
SET(DIR_NAME Demos/3DAPI)
BCI2000_INCLUDE(3DAPI)
bci2000_include(OpenGL)
set(SOURCES
  Demo.cpp
  DemoWindow.cpp
  ${PROJECT_SRC_DIR}/shared/gui/GUI.cpp
  ${PROJECT_SRC_DIR}/shared/gui/GraphDisplay.cpp
  ${PROJECT_SRC_DIR}/shared/gui/GraphObject.cpp
  ${PROJECT_SRC_DIR}/shared/gui/DisplayClock.cpp
  ${PROJECT_SRC_DIR}/shared/gui/Bitmap.cpp
  ${PROJECT_SRC_DIR}/shared/utils/ClockTick.cpp
)

BCI2000_ADD_TOOLS_GUIAPP(
  "${EXECUTABLE_NAME}"
  ${SOURCES}
  OUTPUT_DIRECTORY ${PROJECT_BUILD_ROOT}/demos
)

ENDIF(BUILD_DEMOS)
