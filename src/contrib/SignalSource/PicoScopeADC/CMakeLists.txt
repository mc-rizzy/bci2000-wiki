###########################################################################
## Authors: Nicholas Luczak
## Description: Build information for the RippleADC source module
###########################################################################

# Use the bci2000_include macro if you need to link with frameworks from /src/extlib:


SET( EXECUTABLE_NAME PicoScopeADC)

if(WIN32 AND MSVC AND PROJECT_TARGET_PROCESSOR STREQUAL x64)
set(	FILES_PROJECT
	${FILES_PROJECT}
	lib/ps5000a.dll
		)
    set( lib ${PROJECT_SRC_DIR}/contrib/SignalSource/PicoScopeADC/lib/ps5000a.lib )
bci2000_add_signal_source_module(
  "${EXECUTABLE_NAME}"
  SUPPORT batch/*=>/batch
  SUPPORT parms/*=>/parms/fragments/amplifiers
  PicoScopeADC.cpp
  inc/ps5000Api.h
  ${FILES_PROJECT}
)
target_link_libraries( ${EXECUTABLE_NAME} ${lib} )

endif()
