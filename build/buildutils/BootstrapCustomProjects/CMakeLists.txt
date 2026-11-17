###########################################################################
## $Id: CMakeLists.txt 9306 2026-03-25 13:46:45Z mellinger $
## Authors: jezhill@gmail.com
## Description: Build information for custom-development bootstrapping tools

SET( tools_
  NewBCI2000Module
  NewBCI2000Filter
  NewBCI2000FilterTool
  NewBCI2000FilterDll
  NewBCI2000Logger
)
FOREACH( tool_ ${tools_} )
  add_subdirectory(${tool_})
ENDFOREACH()
