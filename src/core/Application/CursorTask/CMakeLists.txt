###########################################################################
## $Id: CMakeLists.txt 9552 2026-09-14 14:33:23Z tytbutler $
## Authors: griffin.milsap@gmail.com
## Description: Build information for CursorTask in Qt

bci2000_add_application_module(
  CursorTask
  CursorFeedbackTask.cpp
  FeedbackScene2D.cpp
  FeedbackScene3D.cpp
  FeedbackScene.h
  AudioFeedback.cpp
  SimpleSynth.cpp
  ${PROJECT_SRC_DIR}/extlib/NetworkHID/NetworkHID.cpp
  SUPPORT /prog/images/*
  SUPPORT /parms/examples/CursorTask*.prm
)
