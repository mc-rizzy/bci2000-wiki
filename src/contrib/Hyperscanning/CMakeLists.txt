###########################################################################
## $Id: CMakeLists.txt 8460 2024-11-19 16:52:04Z lingling $
## Description: Build information for BCI2000 and CMake

IF( WIN32 )
# Set directory name in IDE
SET( DIR_NAME "${DIR_NAME}Hyperscanning/" )

ADD_SUBDIRECTORY( HyperscanningTest )
ADD_SUBDIRECTORY( HyperscanningApplicationBase )

ENDIF( WIN32 )