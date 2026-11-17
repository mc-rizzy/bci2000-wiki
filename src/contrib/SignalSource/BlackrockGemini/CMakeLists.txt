IF( WIN32 )

if (PROJECT_TARGET_PROCESSOR STREQUAL amd64)
  set(lib ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/cbsdkx64.lib)
elseif (PROJECT_TARGET_PROCESSOR STREQUAL x86)
  set(lib ${CMAKE_CURRENT_SOURCE_DIR}/lib/x86/cbsdk.lib)
else ()
  return ()
endif ()

# Set the executable name
SET( EXECUTABLE_NAME BlackrockGemini )
INCLUDE_DIRECTORIES( include )
INCLUDE_DIRECTORIES( lib )
# Add the source module to the Makefile or compiler project file:
BCI2000_ADD_SIGNAL_SOURCE_MODULE( 
  ${EXECUTABLE_NAME}
  BlackrockGeminiADC.cpp
  SUPPORT batch/*=>/batch
  SUPPORT parms/*=>/parms/fragments/amplifiers
)

target_link_libraries( ${EXECUTABLE_NAME} ${lib} )

ENDIF( WIN32 )

