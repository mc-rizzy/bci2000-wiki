###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including the SourceFilter
## SETS:
##       SRC_EXTLIB - Required source files
##       HDR_EXTLIB - Required header files
##       INC_EXTLIB - Include directories
##       LIBDIR_EXTLIB - Library directories
##       LIBS_EXTLIB - required libraries


SET( SRC_EXTLIB
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/InterpolationFilter.cpp
)

SET( HDR_EXTLIB
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing/InterpolationFilter.h
)

SET( INC_EXTLIB
  ${PROJECT_SRC_DIR}/shared/modules/signalprocessing
)

SET( LIBDIR_EXTLIB
)

SET( LIBS_EXTLIB
)

# Set success
SET( EXTLIB_OK TRUE )
