###########################################################################
## $Id: IncludeExtension.cmake 8609 2025-02-25 16:11:42Z mellinger $
## Authors: Alexander Belsten (belsten@neurotechcenter.org)

if(WIN32)

IF( PROJECT_TARGET_PROCESSOR STREQUAL amd64 )
	SET( BCI2000_SIGSRC_FILES
   		${BCI2000_SIGSRC_FILES}
   		${BCI2000_EXTENSION_DIR}/EyetrackerLoggerEyeLink.cpp
	)

	SET ( BCI2000_SIGSRC_FILES
		${BCI2000_SIGSRC_FILES}
    		${BCI2000_EXTENSION_DIR}/lib/x64/eyelink_core64.dll
		${BCI2000_EXTENSION_DIR}/lib/x64/eyelink_core_graphics64.dll
		${BCI2000_EXTENSION_DIR}/lib/x64/eyelink_w32_comp64.dll
		${BCI2000_EXTENSION_DIR}/lib/x64/SDL.dll
		${BCI2000_EXTENSION_DIR}/lib/x64/SDL_gfx.dll
		${BCI2000_EXTENSION_DIR}/lib/x64/SDL_mixer.dll
		${BCI2000_EXTENSION_DIR}/lib/x64/SDL_ttf.dll
		${BCI2000_EXTENSION_DIR}/lib/x64/libfreetype-6.dll
		${BCI2000_EXTENSION_DIR}/lib/x64/zlib1.dll
	)
  
  	SET( BCI2000_SIGSRC_LIBS
     		${BCI2000_SIGSRC_LIBS}
     		${BCI2000_EXTENSION_DIR}/lib/x64/eyelink_core64.lib
		${BCI2000_EXTENSION_DIR}/lib/x64/eyelink_core_graphics64.lib
		${BCI2000_EXTENSION_DIR}/lib/x64/eyelink_w32_comp64.lib
		${BCI2000_EXTENSION_DIR}/lib/x64/SDL.lib
		${BCI2000_EXTENSION_DIR}/lib/x64/SDL_gfx.lib
		${BCI2000_EXTENSION_DIR}/lib/x64/SDL_mixer.lib
		${BCI2000_EXTENSION_DIR}/lib/x64/SDL_ttf.lib
  	)
ELSEIF( PROJECT_TARGET_PROCESSOR STREQUAL x86 )
	SET( BCI2000_SIGSRC_FILES
   		${BCI2000_SIGSRC_FILES}
   		${BCI2000_EXTENSION_DIR}/EyetrackerLoggerEyeLink.cpp
	)

	SET ( BCI2000_SIGSRC_FILES
		${BCI2000_SIGSRC_FILES}
    		${BCI2000_EXTENSION_DIR}/lib/x86/eyelink_core.dll
		${BCI2000_EXTENSION_DIR}/lib/x86/eyelink_core_graphics.dll
		${BCI2000_EXTENSION_DIR}/lib/x86/eyelink_w32_comp.dll
		${BCI2000_EXTENSION_DIR}/lib/x86/SDL.dll
		${BCI2000_EXTENSION_DIR}/lib/x86/sdl_gfx.dll
		${BCI2000_EXTENSION_DIR}/lib/x86/SDL_mixer.dll
		${BCI2000_EXTENSION_DIR}/lib/x86/SDL_ttf.dll
		${BCI2000_EXTENSION_DIR}/lib/x86/zlib.dll
	)
  
  	SET( BCI2000_SIGSRC_LIBS
     		${BCI2000_SIGSRC_LIBS}
     		${BCI2000_EXTENSION_DIR}/lib/x86/eyelink_core.lib
		${BCI2000_EXTENSION_DIR}/lib/x86/eyelink_core_graphics.lib
		${BCI2000_EXTENSION_DIR}/lib/x86/eyelink_w32_comp.lib
		${BCI2000_EXTENSION_DIR}/lib/x86/SDL.lib
		${BCI2000_EXTENSION_DIR}/lib/x86/sdl_gfx.lib
		${BCI2000_EXTENSION_DIR}/lib/x86/SDL_mixer.lib
		${BCI2000_EXTENSION_DIR}/lib/x86/SDL_ttf.lib
  	)
ELSE()
    message("EyetrackerLoggerEyeLink: Not available for this architecture")
ENDIF()

else()
  message("EyetrackerLoggerEyeLink: Only available on Windows")
endif()
