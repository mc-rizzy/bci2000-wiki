###########################################################################
## Authors:
## Description: Build information for the LSLSource source module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:
# bci2000_include( MATH )

set(DYLIBS
)

if (WIN32)
  if (PROJECT_TARGET_PROCESSOR STREQUAL x86)
    list(APPEND DYLIBS
       extlib/dylib/windows-x86/lsl.dll
    )
  elseif (PROJECT_TARGET_PROCESSOR STREQUAL amd64)
    list(APPEND DYLIBS
       extlib/dylib/windows-amd64/lsl.dll
    )
  endif()
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    # lsl.dll truncates pointers to 32bit,
    # so we must disable "high entropy" addresses.
    # see: https://devblogs.microsoft.com/oldnewthing/20150709-00/?p=45181
    add_link_options("/HIGHENTROPYVA:NO")
  endif()
endif()

bci2000_include(SourceFilter)
bci2000_add_signal_source_module(
  LSLSource
  LSLADC.cpp
  extlib/include/lsl.imports.cpp
  batch/CursorTask_LSLSource.bat
  batch/P3Speller_LSLSource.bat
  batch/StimulusPresentation_LSLSource.bat
  ${DYLIBS}
)

