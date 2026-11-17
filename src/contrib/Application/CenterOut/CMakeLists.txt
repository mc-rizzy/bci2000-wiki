###########################################################################
## $Id: CMakeLists.txt 8610 2025-02-26 16:10:03Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Build information for CursorTask in Qt

bci2000_add_application_module(
  CenterOut
  CenterOutFeedbackTask.cpp
  CursorFeedbackTask.cpp
  FeedbackScene2D.cpp
  FeedbackScene3D.cpp
  FeedbackScene.h
  SUPPORT /prog/images/*
  SUPPORT /parms/examples/CursorTask*.prm
)
