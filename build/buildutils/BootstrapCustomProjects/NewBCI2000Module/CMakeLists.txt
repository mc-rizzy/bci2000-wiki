###########################################################################
## $Id: CMakeLists.txt 9210 2025-12-21 19:11:37Z mellinger $
## Authors: jezhill@gmail.com
## Description: Build information for custom-development bootstrapping tools

SET( DIR_NAME Build/BootstrapCustomProjects )

file( GLOB files ../templates/*.in )
foreach( file_ ${files} )
  get_filename_component( name_ ${file_} NAME )
  string( REGEX REPLACE "(\\.|\\-)" "_" name_ ${name_} )
  utils_add_resource( ${name_} ${file_} )
endforeach()

utils_include( frameworks/Core )
BCI2000_ADD_TARGET(
  INFO "Custom bootstrapping tool"
  CONSOLEAPP NOVERSION NOBUNDLE NewBCI2000Module
  ../BootstrapCustomProjects.cpp
  ${BUILDUTILS_DIR}/buildenv/buildenv.h
  OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  # That's right: these tools are designed to be within easy reach
  # of the same user who then launches "Make * Project Files.bat"    
)

