###########################################################################
## $Id: CMakeLists.txt 9159 2025-11-03 15:13:12Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the ParallelSwitch example

IF( BUILD_DEMOS )

  # Set the executable name
  SET( NAME SimpleWatchesExample )
  set( DIR_NAME Demos/BCI2000Remote )

  # Set the project specific sources
  SET( SRC_PROJECT
    SimpleWatchesExample.cpp
    ${PROJECT_SRC_DIR}/shared/utils/Lib/SockStream.cpp
    ${PROJECT_SRC_DIR}/shared/utils/Lib/SelfPipe.cpp
    ${PROJECT_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp
    ${PROJECT_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp
  )

  SET_OUTPUT_DIRECTORY( ${PROJECT_BUILD_ROOT}/demos )
  BCI2000_ADD_TARGET( INFO "Watches example" CONSOLEAPP ${NAME} ${SRC_PROJECT} )

  IF( WIN32 AND NOT FAILED )
    TARGET_LINK_LIBRARIES( ${NAME} ws2_32 )
  ENDIF()

ENDIF( BUILD_DEMOS )
