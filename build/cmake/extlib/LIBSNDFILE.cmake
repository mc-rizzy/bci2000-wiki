###########################################################################
## $Id: LIBSNDFILE.cmake 8584 2025-02-06 16:00:18Z mellinger $
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including libsndfile in a project

if(WIN32)

  set( EXTLIB_OK TRUE )
  set( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/libsndfile/include" )
  file( GLOB importfiles_ "${INC_EXTLIB}/../imports/*" )
  list( APPEND SRC_EXTLIB ${importfiles_} )
  if( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
    set( libdir_ "${INC_EXTLIB}/../win32-amd64/dynamic" )
  elseif( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
    set( libdir_ "${INC_EXTLIB}/../win32-x86/dynamic" )
  elseif( PROJECT_TARGET_PROCESSOR STREQUAL arm64 )
    set( libdir_ "${INC_EXTLIB}/../win32-arm64/dynamic" )
  else()
    utils_warn("Unsupported architechture: ${PROJECT_TARGET_PROCESSOR}")
    set( EXTLIB_OK FALSE )
  endif()
  file( GLOB dlls_ "${libdir_}/*.dll" )
  foreach( _R0 ${dlls_} )
    list( APPEND SRC_EXTLIB COMPANION ${_R0} )
  endforeach()

else()

  set(lib_sf)
  find_library(lib_sf sndfile NO_CACHE)
  set(inc_sf)
  find_path(inc_sf sndfile.h NO_CACHE)
    
  if(NOT lib_sf)
    set(inc_sf "${PROJECT_SRC_DIR}/extlib/homebrew/include/")
    set(lib_sf "${PROJECT_SRC_DIR}/extlib/homebrew/lib/libsndfile.a")
    utils_warn("Libsndfile could not be found by CMake. Using the portable homebrew instead")
  endif()
  
  set(LIBS_EXTLIB ${lib_sf})
  set(INC_EXTLIB ${inc_sf})
  
  set(EXTLIB_OK true)

endif()
