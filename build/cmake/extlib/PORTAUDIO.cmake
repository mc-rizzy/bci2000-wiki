###########################################################################
## $Id: PORTAUDIO.cmake 8584 2025-02-06 16:00:18Z mellinger $
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including PortAudio in a project

if(WIN32)

  set( EXTLIB_OK TRUE )
  set( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/portaudio/include" )
  file( GLOB importfiles_ "${INC_EXTLIB}/../imports/*" )
  list( APPEND SRC_EXTLIB ${importfiles_} )
  if( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
    set( libdir_ "${INC_EXTLIB}/../win32-amd64/dynamic" )
  elseif( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
    set( libdir_ "${INC_EXTLIB}/../win32-x86/dynamic" )
  elseif( PROJECT_TARGET_PROCESSOR STREQUAL arm64 )
    set( libdir_ "${INC_EXTLIB}/../win32-arm64/dynamic" )
  else()
    utils_warn("Unsupported architecture")
    set( EXTLIB_OK TRUE )
  endif()
  file( GLOB dlls_ "${libdir_}/*.dll" )
  foreach( _R0 ${dlls_} )
    list( APPEND SRC_EXTLIB COMPANION ${_R0} )
  endforeach()

else()

  set(lib_pa)
  find_library(lib_pa portaudio NO_CACHE)
  set(inc_pa)
  find_path(inc_pa portaudio.h NO_CACHE)

  if(NOT lib_pa)
    set(inc_pa "${PROJECT_SRC_DIR}/extlib/homebrew/include/")
    set(lib_pa "${PROJECT_SRC_DIR}/extlib/homebrew/lib/libportaudio.a")
    message( "Portaudio could not be found by CMake. Using the portable homebrew location instead")
  endif()

  set(LIBS_EXTLIB ${lib_pa})
  if(APPLE)
    list(APPEND LIBS_EXTLIB
       "-framework CoreServices"
       "-framework CoreFoundation"
       "-framework AudioUnit"
       "-framework AudioToolbox"
       "-framework CoreAudio"
    )
  endif()
  set(INC_EXTLIB ${inc_pa})

  set( EXTLIB_OK true )

endif()
