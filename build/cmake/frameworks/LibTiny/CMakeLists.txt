###########################################################################
## $Id: CMakeLists.txt 8489 2024-12-04 17:37:26Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Defines a static library containing LibTiny utils.

UTILS_INCLUDE( frameworks/LibTiny/Files )
bci2000_include(ConvertUTF)
IF( WIN32 )
  SET( DEPLIBS ws2_32 winmm )
ENDIF() 
IF( NOT APPLE AND NOT WIN32 )
  SET( DEPLIBS rt pthread dl )
ENDIF()

add_subdirectory( StaticCrt )
add_subdirectory( DynamicCrt )
