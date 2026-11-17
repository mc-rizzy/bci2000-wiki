###########################################################################
## $Id: CMakeLists.txt 9548 2026-09-12 11:09:47Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for BCI2000 and CMake

# Set this directory name
SET( DIR_NAME "${DIR_NAME}ExternalLinks/" )

# Recurse down into all project subdirectories
ADD_SUBDIRECTORY( BCI2000Automation )
ADD_SUBDIRECTORY( BCI2000Command )
ADD_SUBDIRECTORY( BCI2000PresentationLink )
ADD_SUBDIRECTORY( BCI2000PythonBindings )
ADD_SUBDIRECTORY( PyffApplication )
ADD_SUBDIRECTORY( BCI2000RemoteDemos )
ADD_SUBDIRECTORY( Simulink )
