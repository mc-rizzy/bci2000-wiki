###########################################################################
## Authors: juergen@VBox-Win10.fritz.box
## Description: Build information for the BufferedADCTest source module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( MATH )

bci2000_add_signal_source_module(
  BufferedADCTest NOBUNDLE
  SUPPORT batch/*=>/batch
  SUPPORT parms/*=>/parms/fragments/amplifiers
  BufferedADCTestADC.cpp
)
