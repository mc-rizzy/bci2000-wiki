###########################################################################
## $Id: FFT.cmake 8932 2025-07-01 18:33:03Z mellinger $
## Authors: griffin.milsap@gmail.com
## Description: Sets up CMAKE variables for including the FFT lib
## SETS:
##       SRC_EXTLIB - Required source files for the fft library
##       HDR_EXTLIB - Required header files for the fft library
##       INC_EXTLIB - Include directory for the fft library

if (WIN32)

    SET( EXTLIB_OK TRUE )
    # Define the source files
    SET( SRC_EXTLIB
      ${PROJECT_SRC_DIR}/extlib/fftlib/FFTLibWrap.cpp
      ${PROJECT_SRC_DIR}/extlib/fftlib/fftw3/include/fftw3.imports.cpp
    )
    # Define the headers
    SET( HDR_EXTLIB
      ${PROJECT_SRC_DIR}/extlib/fftlib/FFTLibWrap.h
    )

    # Define the include directory
    SET( INC_EXTLIB ${PROJECT_SRC_DIR}/extlib/fftlib )
    if( WIN32 )
      utils_get_extlib( fftw_ fftw 3.3 )
      if( fftw_ )
        list( APPEND SRC_EXTLIB COMPANION ${fftw_}/libfftw3-3.dll=libfftw3-3_${UTILS_ARCH_BITS}.dll )
      else()
        set( EXTLIB_OK FALSE )
      endif()
    endif()

else()

    # Define the source files
    SET( SRC_EXTLIB
      ${PROJECT_SRC_DIR}/extlib/fftlib/FFTLibWrap.cpp
    )
    # Define the headers
    SET( HDR_EXTLIB
      ${PROJECT_SRC_DIR}/extlib/fftlib/FFTLibWrap.h
    )

    # Define include directories
    set(INC_EXTLIB)
    find_path(INC_EXTLIB fftw3.h
      PATHS /usr/local/include
            /opt/homebrew/include
            /home/linuxbrew/.linuxbrew/include
            ${PROJECT_SRC_DIR}/extlib/homebrew/include
      NO_CACHE
    )
    if (NOT INC_EXTLIB)
      set(INC_EXTLIB ${PROJECT_SRC_DIR}/extlib/homebrew/include)
      message( "FFTW3 could not be found by CMake -- Using the portable homebrew location instead. Please run install_dependencies, if you have not already." )
    else()
      cmake_path(GET INC_EXTLIB PARENT_PATH INC_EXTLIB)
    endif()
    list(APPEND INC_EXTLIB ${PROJECT_SRC_DIR}/extlib/fftlib/)
  
    # We use full paths to libraries because LIBDIRS_EXTLIB will not be
    # propagated from libraries to consuming executables
    set(lib)
    find_library (lib fftw3
      PATHS /usr/local/lib
            /opt/homebrew/lib
            /home/linuxbrew/.linuxbrew/lib
            ${PROJECT_SRC_DIR}/extlib/homebrew/lib
      NO_CACHE
    )
    # link statically on macOS to avoid strange issues with dependencies
    if (APPLE)
      cmake_path(REPLACE_EXTENSION lib LAST_ONLY ".a")
    endif()
    list (APPEND LIBS_EXTLIB ${lib})
  
    if (INC_EXTLIB AND LIBS_EXTLIB)
      set( EXTLIB_OK true )
    endif()
  
endif()
