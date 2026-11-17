###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org
## Description: Sets up CMAKE variables for including g.NEEDaccess in a project

if (NOT MSVC OR PROJECT_TARGET_PROCESSOR STREQUAL arm64)
  utils_warn("g.NEEDaccess libraries are only available for x86 and x64 MSVC on Windows.")
  set(EXTLIB_OK false)
  return()
endif()

set(INC_EXTLIB "${PROJECT_SRC_DIR}/extlib/gtec/gNEEDaccess")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(dlldir_ x64)
else()
  set(dlldir_ win32)
endif()
set(dlldir_ ${INC_EXTLIB}/${dlldir_})
file(GLOB dlls_ "${dlldir_}/*.dll")
foreach (dll_ ${dlls_})
  list(APPEND SRC_EXTLIB COMPANION ${dll_})
endforeach()
list(APPEND LIBS_EXTLIB "${dlldir_}/GDSClientAPI.lib")

set( EXTLIB_OK TRUE )

