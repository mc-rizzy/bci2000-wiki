###########################################################################
## $Id: CMakeLists.txt 9552 2026-09-14 14:33:23Z tytbutler $
## Authors: griffin.milsap@gmail.com
## Description: Build information for P3Speller in Qt

BCI2000_ADD_APPLICATION_MODULE( 
  P3Speller
  P3SpellerTask.cpp
  ${PROJECT_SRC_DIR}/extlib/NetworkHID/NetworkHID.cpp
  SUPPORT /prog/images/*
  SUPPORT /parms/examples/P3Speller*.prm
)
