###########################################################################
## $Id: CMakeLists.txt 9485 2026-07-21 17:16:08Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for OperatorQt

# Set the executable name
SET( EXECUTABLE_NAME Operator )

# Set the project specific sources
SET( SRC_PROJECT
  main.cpp 
  MainWindow.ui
  MainWindow.cpp
  ConnectionInfo.cpp
  ShowStates.cpp
  WatchesWindow.cpp
  VisualizationDock.cpp
  NotesWindow.cpp
  VisRecorder.cpp
  SysLog.cpp
  CustomDialog.cpp

  ${BCI2000_PARAMETER_GUI}

  ../OperatorLib/src/ScriptParser/ParserToken.cpp
  ScriptingExtensions/ScriptingExtensions.cpp
  ScriptingExtensions/WindowType.cpp
  ScriptingExtensions/TitleType.cpp
  ScriptingExtensions/ButtonType.cpp
  ScriptingExtensions/WatchType.cpp
  ScriptingExtensions/VisualizationType.cpp
  ScriptingExtensions/NoteType.cpp
  ScriptingExtensions/NamedPipeType.cpp
  ScriptingExtensions/TelnetType.cpp
  ScriptingExtensions/WebsocketType.cpp
  ScriptingExtensions/FileSystemTypes.cpp
  ScriptingExtensions/DialogType.cpp

  Visualizations/SignalWidget.cpp
  Visualizations/VisDisplay.cpp
  Visualizations/VisDisplayWindow.cpp
  Visualizations/VisDisplayContainer.cpp
  Visualizations/VisDisplayLayer.cpp
  Visualizations/VisDisplayBase.cpp
  Visualizations/VisDisplayMemo.cpp
  Visualizations/VisDisplayGraph.cpp
  Visualizations/VisDisplaySpectrum.cpp
  Visualizations/VisDisplayBitmap.cpp
  Visualizations/VisDisplayVideoFrame.cpp

  ${PROJECT_SRC_DIR}/shared/utils/AV/AVEncoder.cpp
  ${PROJECT_SRC_DIR}/shared/utils/AV/AVError.cpp
  ${PROJECT_SRC_DIR}/shared/utils/AV/AVLog.cpp
  ${PROJECT_SRC_DIR}/shared/utils/DisplayFiltering/DisplayFilter.cpp
  ${PROJECT_SRC_DIR}/shared/utils/DisplayFiltering/DecimationFilter.cpp
  ${PROJECT_SRC_DIR}/shared/utils/DisplayFiltering/ScaleObservationFilter.cpp
  ${PROJECT_SRC_DIR}/shared/bcistream/BCIStream_guiapp.cpp
  ${PROJECT_SRC_DIR}/shared/gui/SignalDisplay.cpp
  ${PROJECT_SRC_DIR}/shared/gui/AboutBox.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Settings.cpp
  ${PROJECT_SRC_DIR}/shared/utils/LogEntry.cpp
  ${BCI2000_SRC_DIR}/shared/utils/Qt/ExtendedColorSelector.cpp
  ${PROJECT_SRC_DIR}/shared/utils/Qt/QtSignalConnector.h
  ${PROJECT_SRC_DIR}/shared/utils/Qt/QtWidgetProxy.h

  ${PROJECT_SRC_DIR}/shared/utils/Qt/QtMain.cpp
)
include_directories( ${PROJECT_SRC_DIR}/core/Operator/OperatorLib/include/ )


if( WIN32 )
  if( USE_APPLICATION_BUNDLES )
    set( OPERATOR_LIB ${PROJECT_BUILD_ROOT}/prog/OperatorLib.dll )
    list( APPEND SRC_PROJECT ${OPERATOR_LIB} )
    set_source_files_properties( ${OPERATOR_LIB} PROPERTIES GENERATED TRUE )
  endif()
endif()

if( APPLE )
  list(APPEND SRC_PROJECT
    Info.plist
  )
endif()

bci2000_include( math )
bci2000_include( ffmpeg )
utils_include( frameworks/Core )
utils_include( frameworks/LibTiny )
bci2000_add_to_inventory( Operator ${EXECUTABLE_NAME} )
utils_set_appicon( gui-screen;signal-document )
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
utils_add_resource(logarithmic_icon ${PROJECT_SRC_DIR}/shared/gui/images/toolbar-icons/logarithmic_96.png)

bci2000_add_target(
  INFO "Operator module" QTAPP ${EXECUTABLE_NAME}
  ${SRC_PROJECT}
  LIBRARY OperatorLib
)

