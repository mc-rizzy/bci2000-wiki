if (WIN32)
    set (dylib_extension dll)
    set (lib_extension lib)
elseif (APPLE)
    set (dylib_extension dylib)
    set (lib_extension dylib)
else ()
    set (dylib_extension so)
    set (lib_extension so)
endif ()

include_directories(${PROJECT_BUILD_ROOT}/prog)

BCI2000_ADD_TARGET(CONSOLEAPP
  SignalSharingClientLibDemo
  SignalSharingClientLibDemo.cpp
  DEPENDS SignalSharingClientLib
#  SUPPORT ../SignalSharingDemo.bat=>/batch
  OUTPUT_DIRECTORY ${PROJECT_BUILD_ROOT}/demos
  COMPANION ${PROJECT_BUILD_ROOT}/prog/SignalSharingClientLib.${dylib_extension}
)

target_link_libraries(SignalSharingClientLibDemo ${PROJECT_BUILD_ROOT}/prog/SignalSharingClientLib.${lib_extension})

