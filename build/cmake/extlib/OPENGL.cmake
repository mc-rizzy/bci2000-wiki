###########################################################################
## $Id: OPENGL.cmake 8664 2025-03-28 14:58:51Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including OpenGL in a project

SET( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/openGL" )
SET( LIBS_EXTLIB )

set( SRC_EXTLIB
  ${INC_EXTLIB}/OpenGLContext.cpp
)
IF( WIN32 )
  SET( LIBS_EXTLIB
      ${LIBS_EXTLIB}
      d3d9
      opengl32
  )
elseif(APPLE)
  list(APPEND SRC_EXTLIB
    ${INC_EXTLIB}/CGLGlue.mm
  )
  set( LIBS_EXTLIB "-framework OpenGL" )
ELSE()
  SET( LIBS_EXTLIB -lGL -lGLU )
ENDIF()

SET( EXTLIB_OK TRUE )
