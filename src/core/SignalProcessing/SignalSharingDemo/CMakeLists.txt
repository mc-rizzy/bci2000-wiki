if(BUILD_DEMOS)

  set(DIR_NAME Demos/SignalSharing)
  add_subdirectory(CppClientApp)
  add_subdirectory(CppLibraryClient)
  add_subdirectory(MatlabClient)

endif()
