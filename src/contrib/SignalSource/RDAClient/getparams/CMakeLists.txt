###########################################################################
## $Id: CMakeLists.txt 8640 2025-03-17 17:21:13Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Build information for Neuroscan getparams tool

SET( NAME RDAgetparams )

SET( SOURCES
  RDAgetparams.cpp
  ${BCI2000_SRC_DIR}/contrib/SignalSource/RDAClient/RDAProtocol.cpp
  ${BCI2000_SRC_DIR}/shared/bcistream/BCIStream_tool.cpp
)

UTILS_INCLUDE( frameworks/Core )
utils_set_appicon( wrench )
BCI2000_ADD_TARGET( INFO Tool EXECUTABLE ${NAME} ${SOURCES} )
BCI2000_ADD_TO_INVENTORY( Tool ${NAME} )
