###########################################################################
## Authors: lingl@DESKTOP-GH0R7CA.wucon.wustl.edu
## Description: Build information for the EglyDriverTask application module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( 3DAPI )
IF( WIN32 )

bci2000_add_application_module(
  EglyDriverTask
  EglyDriverTask.cpp
  SUPPORT batch/*=>/batch
  SUPPORT parms/EglyDriverTask/*=>/parms/EglyDriverTask
)

ENDIF( WIN32 )