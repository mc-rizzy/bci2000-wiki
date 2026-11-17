###########################################################################
## $Id: PTHREADS.cmake 8584 2025-02-06 16:00:18Z mellinger $
## Author: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including the pthreads-win32 API
## SETS:
##       SRC_EXTLIB - Required source files for pthreads
##       HDR_EXTLIB - Required header files for pthreads
##       INC_EXTLIB - Include directory for pthreads
##       LIBDIR_EXTLIB - Library directory for pthreads
##       LIBS_EXTLIB - required library for pthreads

IF( WIN32 )

# Set the Source and headers
SET( SRC_EXTLIB )
SET( HDR_EXTLIB
  ${PROJECT_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/pthread.h
  ${PROJECT_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/sched.h
  ${PROJECT_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include/semaphore.h
)

# Define the include directory
SET( INC_EXTLIB 
  ${PROJECT_SRC_DIR}/extlib/fieldtrip/pthreads-win32/include
)

# Define where the library is
IF( PROJECT_TARGET_PROCESSOR STREQUAL amd64 ) 
  SET( LIBDIR_EXTLIB ${PROJECT_SRC_DIR}/extlib/fieldtrip/pthreads-win32/lib/x64 )
ELSEIF( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
  SET( LIBDIR_EXTLIB ${PROJECT_SRC_DIR}/extlib/fieldtrip/pthreads-win32/lib/x86 )
ELSE()
  utils_warn("Unsupported architecture: ${PROJECT_TARGET_PROCESSOR}")
  set(EXTLIB_OK FALSE)
  return()
ENDIF()

# Set Libs required
IF( MINGW )
  SET( LIBS_EXTLIB libpthreadGC2.a )
ELSEIF( MSVC )
  SET( LIBS_EXTLIB pthreadVC2.lib )
  if( MSVC_VERSION GREATER 1800 )
    add_definitions( -DHAVE_STRUCT_TIMESPEC )
  endif()
ENDIF()

# Set success
SET( EXTLIB_OK TRUE )

ENDIF()
