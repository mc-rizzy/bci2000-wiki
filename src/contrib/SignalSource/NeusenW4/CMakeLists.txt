###########################################################################
## Authors: mellinger@neurotechcenter.org
## Description: Build information for the NeusenW4 source module
###########################################################################

LINK_LIBRARIES(${CMAKE_CURRENT_SOURCE_DIR}/NeusenWSDK/NeusenW4SDKDll.lib)
INCLUDE_DIRECTORIES(NeusenWSDK)
BCI2000_INCLUDE(SourceFilter)

IF (PROJECT_TARGET_PROCESSOR STREQUAL amd64 AND WIN32 AND MSVC) 

bci2000_add_signal_source_module(
  NeusenW4
  NeusenW4ADC.cpp

  SUPPORT batch/*=>/batch
  SUPPORT parms/*=>/parms/fragments/amplifiers
  COMPANION NeusenWSDK/aEEGCoeff.bin
  COMPANION NeusenWSDK/filterCoeff.bin
  COMPANION NeusenWSDK/filterCoeff_fir.bin

  NeusenWSDK/vcruntime140.dll
  NeusenWSDK/vcruntime140_1.dll
  NeusenWSDK/msvcr110.dll

  NeusenWSDK/filter.dll
  NeusenWSDK/hasp_windows_x64_112370.dll
  NeusenWSDK/ippcore.dll
  NeusenWSDK/ipps.dll
  NeusenWSDK/ippsl9.dll
  NeusenWSDK/libpthread-stdcall.dll
  NeusenWSDK/liquid-dsp.dll
  NeusenWSDK/NeuracleDsp.dll
  NeusenWSDK/Neuracle.Dongle.dll
  NeusenWSDK/libliquid.dll
  NeusenWSDK/libfftw3-3.dll
  NeusenWSDK/NeusenW4SDKDll.dll
)
ELSE()
  MESSAGE( "--- NeusenW v4 libaries not available for this platform. Will not build." )
ENDIF()
