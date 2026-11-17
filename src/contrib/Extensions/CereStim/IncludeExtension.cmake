###########################################################################
## $Id: IncludeExtension.cmake 8584 2025-02-06 16:00:18Z mellinger $
## Authors: Alexander Belsten (belsten@neurotechcenter.org)


SET( OK FALSE )

IF( WIN32 )

  IF( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
    # Set the project specific sources
    SET( BCI2000_APP_FILES
      ${BCI2000_APP_FILES}
      ${BCI2000_EXTENSION_DIR}/CereStim.cpp
      ${BCI2000_EXTENSION_DIR}/lib/x64/BStimAPIx64.dll
    )
    list( APPEND BCI2000_APP_LIBS 
			${CMAKE_CURRENT_LIST_DIR}/lib/x64/BStimAPIx64.lib
		)
    SET (OK TRUE)  
    
  ELSEIF( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
    # Set the project specific sources
    SET( BCI2000_APP_FILES
      ${BCI2000_APP_FILES}
      ${BCI2000_EXTENSION_DIR}/CereStim.cpp
      ${BCI2000_EXTENSION_DIR}/lib/x86/BStimAPIx86.dll
    )
    list( APPEND BCI2000_APP_LIBS 
			${CMAKE_CURRENT_LIST_DIR}/lib/x86/BStimAPIx86.lib
		)
    SET (OK TRUE)

  ENDIF()

ENDIF()

IF( NOT OK )
  MESSAGE( "--- CereStim libraries not available for this platform. Will not build." )
ENDIF()


