###########################################################################
## $Id: FFMPEG.cmake 9189 2025-12-10 17:19:38Z mellinger $
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Sets up CMAKE variables for including ffmpeg-LibAV in a project

if(WIN32)

  set( EXTLIB_OK TRUE )
  set( INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/ffmpeg/include" )
  file( GLOB libavfiles_ "${INC_EXTLIB}/../imports/*" )
  list( APPEND SRC_EXTLIB ${libavfiles_} )
  if( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
    set( libdir_ "${INC_EXTLIB}/../win32-amd64/dynamic" )
  elseif( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
    set( libdir_ "${INC_EXTLIB}/../win32-x86/dynamic" )
  elseif( PROJECT_TARGET_PROCESSOR STREQUAL arm64 )
    set( libdir_ "${INC_EXTLIB}/../win32-arm64/dynamic" )
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
  find_path(INC_EXTLIB avcodec.h
    PATH_SUFFIXES libavcodec
    PATHS /usr/local/include
          /opt/homebrew/include
          /home/linuxbrew/.linuxbrew/include
          ${PROJECT_SRC_DIR}/extlib/homebrew/include
    NO_CACHE
    NO_DEFAULT_PATH
  )
  if (NOT INC_EXTLIB)
    # This time with default path only
    find_path(INC_EXTLIB avcodec.h
      PATH_SUFFIXES libavcodec
      NO_CACHE
    )
  endif()
  if (NOT INC_EXTLIB)
    set(INC_EXTLIB ${PROJECT_SRC_DIR}/extlib/homebrew/include)
    message( "FFMPEG could not be found by CMake -- Using the portable homebrew location instead. Please run install_dependencies, if you have not already." )
  else()
    cmake_path(GET INC_EXTLIB PARENT_PATH INC_EXTLIB)
    cmake_path(GET INC_EXTLIB PARENT_PATH INC_EXTLIB)
  endif()
  
  # We use full paths to libraries because LIBDIRS_EXTLIB will not be
  # propagated from libraries to consuming executables
  set (libnames avformat avcodec avdevice swresample swscale avutil)
  foreach (libname ${libnames})
    set(lib)
    find_library (lib ${libname}
      PATHS /usr/local/lib
            /opt/homebrew/lib
            /home/linuxbrew/.linuxbrew/lib
            ${PROJECT_SRC_DIR}/extlib/homebrew/lib
      NO_CACHE
      NO_DEFAULT_PATH
    )
    if (NOT lib)
      # This time with default path only
      find_library (lib ${libname}
        NO_CACHE
      )
    endif()
    list (APPEND LIBS_EXTLIB ${lib})
  endforeach()

  if (INC_EXTLIB AND LIBS_EXTLIB)
    set( EXTLIB_OK true )
  endif()
  
  if (NOT APPLE)
    list (APPEND LIBS_EXTLIB -lz -ldrm)
  endif()

endif()

