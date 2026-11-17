###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including ZeroMQ in a project

if(WIN32)

  set( EXTLIB_OK TRUE )
  set( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/libzmq/include" )
  file( GLOB zmqfiles_ "${INC_EXTLIB}/../imports/*" )
  list( APPEND SRC_EXTLIB ${zmqfiles_} )
  if( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
    set( libdir_ "${INC_EXTLIB}/../win32-amd64/dynamic" )
  else()
    utils_warn("Unsupported architecture: ${PROJECT_TARGET_PROCESSOR}")
    set( EXTLIB_OK FALSE )
  endif()
  file( GLOB dlls_ "${libdir_}/*.dll" )
  foreach( _R0 ${dlls_} )
    list( APPEND SRC_EXTLIB COMPANION ${_R0} )
  endforeach()

else()

  set(INC_EXTLIB)
  find_path(INC_EXTLIB zmq.h
    PATHS /usr/local/include
          /opt/homebrew/include
          /home/linuxbrew/.linuxbrew/include
          ${PROJECT_SRC_DIR}/extlib/homebrew/include
    NO_CACHE
  )
  if (NOT INC_EXTLIB)
    set(INC_EXTLIB ${PROJECT_SRC_DIR}/extlib/homebrew/include)
    message( "ZeroMQ could not be found by CMake -- Using the portable homebrew location instead. Please run install_dependencies, if you have not already." )
  endif()
  
  # We use full paths to libraries because LIBDIRS_EXTLIB will not be
  # propagated from libraries to consuming executables
  set (libnames zmq)
  foreach (libname ${libnames})
    set(lib)
    find_library (lib ${libname}
      PATHS /usr/local/lib
            /opt/homebrew/lib
            /home/linuxbrew/.linuxbrew/lib
            ${PROJECT_SRC_DIR}/extlib/homebrew/lib
      NO_CACHE
    )
    list (APPEND LIBS_EXTLIB ${lib})
  endforeach()

  if (INC_EXTLIB AND LIBS_EXTLIB)
    set( EXTLIB_OK true )
  endif()
  
endif()

# Add cppzmq headers
list (APPEND INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/cppzmq")


