###########################################################################
## $Id: CMakeLists.txt 8640 2025-03-17 17:21:13Z mellinger $
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Build information for USBampgetinfo

SET( NAME gUSBampgetinfo )

UTILS_INCLUDE( frameworks/Core )
utils_set_appicon( wrench )
BCI2000_ADD_TARGET( INFO Tool EXECUTABLE ${NAME}
  gUSBampgetinfo.cpp
  ../lib/gUSBamp.imports.cpp
)
IF( NOT FAILED )
  BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
ENDIF( NOT FAILED )
