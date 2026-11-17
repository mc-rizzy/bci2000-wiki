###########################################################################
## $Id: SigSrcFiles.cmake 9495 2026-07-22 15:34:51Z mellinger $
## Authors: griffin.milsap@gmail.com, juergen.mellinger@uni-tuebingen.de

set( SRC_BCI2000_FRAMEWORK
  ${SRC_BCI2000_FRAMEWORK}

  ${PROJECT_SRC_DIR}/shared/fileio/FileWriterBase.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/OutputFileStream.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/FileSplittingCondition.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/GenericFileWriter.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/GenericOutputFormat.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/NullFileWriter.cpp

  ${PROJECT_SRC_DIR}/shared/fileio/dat/BCI2000FileWriter.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/dat/BCI2000OutputFormat.cpp

  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/EDFHeader.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/EDFFileWriter.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/EDFOutputBase.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/EDFOutputFormat.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/GDF.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/GDFFileWriter.cpp
  ${PROJECT_SRC_DIR}/shared/fileio/edf_gdf/GDFOutputFormat.cpp

  ${PROJECT_SRC_DIR}/shared/modules/signalsource/BufferedADC.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/GenericADC.h

  ${PROJECT_SRC_DIR}/shared/modules/signalsource/AlignmentFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/DataIOFilter.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/SourceFilterChain.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/TransmissionFilter.cpp

  ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/EventLink.cpp
  ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/RefLogger.cpp

  ${PROJECT_SRC_DIR}/shared/utils/Multithreading.cpp
)

if (WIN32)
  list( APPEND SRC_BCI2000_FRAMEWORK
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/JoystickLogger_win32.cpp
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/KeyLogger_win32.cpp
  )
endif()
if (APPLE)
  list( APPEND SRC_BCI2000_FRAMEWORK
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/JoystickLogger_macOS.cpp
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/KeyLogger_macOS.cpp
  )
endif()
if (LINUX)
  list( APPEND SRC_BCI2000_FRAMEWORK
    ${PROJECT_SRC_DIR}/shared/modules/signalsource/logging/KeyLogger_linux.cpp
  )
endif()
