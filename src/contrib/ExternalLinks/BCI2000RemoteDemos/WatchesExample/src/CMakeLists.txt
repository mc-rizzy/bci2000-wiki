###########################################################################
## $Id: CMakeLists.txt 9159 2025-11-03 15:13:12Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for the BCI2000Remote Watches example

IF(BUILD_DEMOS)

# Set the executable name
SET(NAME WatchesExample)
set(DIR_NAME Demos/BCI2000Remote)

# Set the project specific sources
SET(SRC_PROJECT
    main.cpp
    mainUI.cpp
    ${PROJECT_SRC_DIR}/shared/utils/Qt/QtMain.cpp # required for our adapted Qt deployment
    ${PROJECT_SRC_DIR}/shared/types/Blob.cpp # required for our adapted Qt deployment

    # These source files are required to use the BCI2000Remote class.
    ${PROJECT_SRC_DIR}/shared/utils/Lib/SockStream.cpp
    ${PROJECT_SRC_DIR}/shared/utils/Lib/SelfPipe.cpp
    ${PROJECT_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Connection.cpp
    ${PROJECT_SRC_DIR}/core/Operator/BCI2000Remote/BCI2000Remote.cpp
)

SET_OUTPUT_DIRECTORY(${PROJECT_BUILD_ROOT}/demos)
UTILS_INCLUDE(frameworks/LibTiny) # required for our adapted Qt deployment
BCI2000_ADD_TARGET(QTAPP ${NAME} ${SRC_PROJECT})
IF(WIN32 AND NOT FAILED)
  TARGET_LINK_LIBRARIES(${NAME} ws2_32)
ENDIF()

ENDIF(BUILD_DEMOS)
