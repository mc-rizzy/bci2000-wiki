###########################################################################
## $Id: ToolsMexfileMacro.cmake 8688 2025-04-08 18:39:49Z mellinger $
## Authors: juergen.mellinger@neurotechcenter.org
## Description: Create a mex file

option( BUILD_MEX_FILES "Build Matlab mex files" OFF )

macro( bci2000_add_mexfile )
  utils_parse_args( "NAME;SOURCES" ${ARGV} )

  if( BUILD_MEX_FILES )
    utils_extract_flag(SOURCES NO_COMPANIONS no_companions_)
    utils_get_mex_suffix(mex_suffix_)
    utils_include( frameworks/Core )
    bci2000_include( matlab_mex )
    if (NOT no_companions_)
      set( mex_companions_
        COMPANION test_bcimex.m
        COMPANION testdata.dat
        COMPANION testdata.mat
      )
    endif()
    if( EXISTS "${PROJECT_SRC_DIR}/core/Tools/mex/${NAME}.m" )
      set( mex_companions_
        ${mex_companions_}
        COMPANION "${NAME}.m"
        COMPANION missing_mex_file.m
      )
    endif()
    bci2000_add_target(
      INFO "Matlab mex file"
      DLL ${NAME}
      ${SOURCES}
      ${PROJECT_SRC_DIR}/core/Tools/mex/mexutils.cpp
      ${PROJECT_SRC_DIR}/shared/bcistream/BCIStream_mex.cpp
      OUTPUT_DIRECTORY "${PROJECT_BUILD_ROOT}/tools/mex/"
      ${mex_companions_}
    )
    set_property( TARGET ${NAME} PROPERTY SUFFIX ".${mex_suffix_}" )
  endif()
endmacro()
