###########################################################################
## Authors: Brunnerlab@DESKTOP-F8KRI7F.wucon.wustl.edu
## Description: Build information for the ClosedLoopStim project
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
bci2000_include( FFT )

bci2000_add_signal_processing_module(
  ClosedLoopStim NOBUNDLE
  PipeDefinition.cpp
  ControlStateFilter.cpp
)
