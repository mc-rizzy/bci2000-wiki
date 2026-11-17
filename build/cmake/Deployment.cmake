###########################################################################
## $Id$
## Authors: mellinger@neurotechcenter.org
## Description: CMake deployment functions for various operating systems

# Simply copy executable plus companions to output directory
function(DEPLOY_PLAIN name_ outputDir libraries_ bundle companions)

    cmake_path(ABSOLUTE_PATH outputDir NORMALIZE)

    if (APPLE)
        add_custom_command(
            TARGET ${name_}
            POST_BUILD
            COMMAND codesign --sign - --force --preserve-metadata=entitlements,requirements,flags,runtime "$<TARGET_FILE:${name_}>"
            COMMENT "Sign executable"
            VERBATIM
        )
    endif()

    set(clear_command)
    set(copy_associated_command)
    if (NOT name_ STREQUAL clear_target_location) # don't depend on yourself
        set(clear_command
            COMMAND clear_target_location "$<TARGET_FILE:${name_}>" "${outputDir}"
        )
        set(copy_associated_command
            COMMAND copy_associated_file  .dll .lib "$<TARGET_FILE:${name_}>" "${outputDir}"
        )
    endif()

    ADD_CUSTOM_COMMAND(
        TARGET ${name_}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${outputDir}"
        ${clear_command}
        COMMAND
        "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${name_}>" "${outputDir}"
        || "${CMAKE_COMMAND}" -E remove "$<TARGET_FILE:${name_}>"
        && "${CMAKE_COMMAND}" -E touch_nocreate .notexist
        ${copy_associated_command}
        COMMENT "Copy to destination"
        VERBATIM
    )
    FOREACH( companion_ ${bundle})
      if( companion_ MATCHES ".*=.*" )
        string( REPLACE "=" ";" companion_ ${companion_} )
        list( GET companion_ 0 from_ )
        list( GET companion_ 1 to_ )
      else()
        set( from_ ${companion_} )
        set( to_ "" )
      endif()
      ADD_CUSTOM_COMMAND(
        TARGET ${name_}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${outputDir}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${from_}" "${outputDir}/${to_}"
        VERBATIM
      )
    ENDFOREACH()

endfunction()

# Deploy bundleable executables on Win32
function(DEPLOY_BUNDLEABLE_WIN32 name_ outputDir libraries_ bundle companions)

    set( deps_ )
    set( copy_as_ )
    FOREACH( companion_ ${bundle} )
      if( companion_ MATCHES ".*=.*" )
        string( REPLACE "=" ";" companion_ ${companion_} )
        list( GET companion_ 0 from_ )
        list( GET companion_ 1 to_ )
        # cmd shell treats = character as argument separator
        list( APPEND copy_as_ --copy-as ${from_} ${to_} )
      else()
        list( APPEND deps_ ${companion_} )
      endif()
    ENDFOREACH()
    get_property(link_dirs_ DIRECTORY PROPERTY LINK_DIRECTORIES)
    set(redist_dirs)
    foreach(dir_ ${link_dirs_})
      list( APPEND redist_dirs "--add-redist-dir" ${dir_} )
    endforeach()
    add_dependencies( ${name_} list_dependencies )
    set( deploy_app_ ${BUILDUTILS_DIR}/deploy_app )
    if( USE_APPLICATION_BUNDLES )
      add_dependencies( ${name_} AppBundler )
      set( bundlearg_ "--bundle" )
      set( info_ "Creating application bundle" )
    else()
      add_dependencies( ${name_} AppCopier )
      set( bundlearg_ "" )
      set( info_ "Deploying executable" )
    endif()
    set(copy_libraries_)
    foreach(lib_ ${libraries_})
      if (TARGET ${lib_})
        list(APPEND copy_libraries_
          COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${lib_}>" "$<TARGET_FILE_DIR:${name_}>"
        )
      endif()
    endforeach()
    ADD_CUSTOM_COMMAND(
        TARGET ${name_}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${outputDir}"
        ${copy_libraries_}
        COMMAND # using "call" is mandatory when calling a batch file
        call "${deploy_app_}" "${outputDir}/${name_}" "$<TARGET_FILE:${name_}>" ${bundlearg_} ${redist_dirs} ${deps_} ${copy_as_}
        || "${CMAKE_COMMAND}" -E remove "$<TARGET_FILE:${name_}>"
        && "${CMAKE_COMMAND}" -E touch_nocreate .notexist
        COMMENT ${info_}
        VERBATIM
    )

endfunction()

# Deploy bundleable executables on macOS
function(DEPLOY_BUNDLEABLE_APPLE name_ outputDir libraries_ bundle companions MACDEPLOYQT kind)

    file(REAL_PATH ${outputDir} outputDir)

    add_custom_command(
        TARGET ${name_}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${outputDir}"
        COMMAND
        "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${name_}>" "${outputDir}"
        || "${CMAKE_COMMAND}" -E remove "$<TARGET_FILE:${name_}>"
        && "${CMAKE_COMMAND}" -E touch_nocreate .notexist
        COMMENT "Copy to destination"
    )
    if(USE_MACOS_APPLICATION_BUNDLES)
        set(build_macos_bundle_
            TARGET ${name_}
            POST_BUILD
            COMMAND test -d "${outputDir}/${name_}.app" && rm -r "${outputDir}/${name_}.app" || true
            COMMAND mkdir -p "${outputDir}/${name_}.app/Contents/MacOS"
            COMMAND mkdir -p "${outputDir}/${name_}.app/Contents/Resources"
            COMMAND mv "${outputDir}/${name_}" "${outputDir}/${name_}.app/Contents/MacOS"
        )
        if (kind STREQUAL CONSOLEAPP)
            list(APPEND build_macos_bundle_
                    COMMAND make_macOS_relative_link "${outputDir}/${name_}.app/Contents/MacOS/${name_}" "${outputDir}/${name_}"
            )
        endif()
        set(got_plist FALSE)
        set(icon FALSE) # extract icon from companions
        foreach(companion_ ${companions})
            if(companion_ MATCHES .*\.icns)
                list(APPEND build_macos_bundle_
                    COMMAND cp "${companion_}" "${outputDir}/${name_}.app/Contents/Resources"
                )
                if (NOT icon)
                    set(icon ${companion_})
                endif()
            elseif(companion_ MATCHES .*/Info\.plist)
                set(got_plist TRUE)
                list(APPEND build_macos_bundle_
                    COMMAND cp "${companion_}" "${outputDir}/${name_}.app/Contents"
                )
            else()
                list(APPEND build_macos_bundle_
                    COMMAND cp "${companion_}" "${outputDir}/${name_}.app/Contents/MacOS"
                )
            endif()
        endforeach()

        # Create an Info.plist file from a template if no individual one exists.
        if (NOT got_plist)
            cmake_path(GET icon FILENAME icon)
            set(MakeInfoPlist ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/MakeInfoPlist.cmake)
            list(APPEND build_macos_bundle_
                COMMAND "${CMAKE_COMMAND}" -D "NAME=${name_}" -D "OUT_DIR=${outputDir}/${name_}.app/Contents" -D "ICON=${icon}" -P "${MakeInfoPlist}"
            )
        endif()

        # We need some tool that copies dependencies into the application
        # bundle. Qt provides macdeployqt that is supposed to do that,
        # but it seems to ignore some dependencies, so we use
        # a similar tool called dylibbundler first.
        set(portableHomebrew ${PROJECT_SRC_DIR}/extlib/homebrew)
        set(dylibbundler ${portableHomebrew}/bin/dylibbundler)
        if (NOT EXISTS ${dylibbundler})
            set(dylibbundler $ENV{HOMEBREW_PREFIX}/bin/dylibbundler)
        endif()
        if (NOT EXISTS ${dylibbundler})
            set(dylibbundler dylibbundler)
        endif()
        execute_process(COMMAND which dylibbundler RESULT_VARIABLE error OUTPUT_QUIET)
        if (error)
            set(dylibbundler /opt/homebrew/bin/dylibbundler)
            if (NOT EXISTS ${dylibbundler})
                message(FATAL_ERROR
                    "Need dylibbundler homebrew tool but cannot find it. "
                    "Make sure dylibbundler is installed and is in your PATH."
                )
            endif()
        endif()
        list(APPEND build_macos_bundle_
            COMMAND ${dylibbundler} -cd -ns -b -x "${outputDir}/${name_}.app/Contents/MacOS/${name_}" -d "${outputDir}/${name_}.app/Contents/Frameworks/" -p @executable_path/../Frameworks/ -s /usr/local/lib -s $ENV{HOMEBREW_PREFIX}/lib -s "${portableHomebrew}/lib" -s "${outputDir}" > /dev/null 2>&1
        )
        if(MACDEPLOYQT)
            # Qt's macdeployqt will now copy Qt frameworks, and plugins
            # into the bundle.
            set(nostrip_ -no-strip)
            if(CMAKE_BUILD_TYPE MATCHES ^.elease$)
                set(nostrip_)
            endif()
            list(APPEND build_macos_bundle_
                COMMAND "${MACDEPLOYQT}" "${outputDir}/${name_}.app" ${nostrip_}
            )
        endif()
        list(APPEND build_macos_bundle_
            COMMAND clean_macOS_rpaths "${outputDir}/${name_}.app/Contents/MacOS/${name_}"
        )

        # Create a valid signature for local deployment
        # ("ad-hoc signing", no developer id required).
        # On ARM Macs, signing is necessary to run a program.
        list(APPEND build_macos_bundle_
            COMMAND codesign --remove-signature "${outputDir}/${name_}.app" || true
            COMMAND codesign --force --deep -s - "${outputDir}/${name_}.app"
        )
        list(APPEND build_macos_bundle_
            COMMENT "Deploying ${name_} for macOS"
            VERBATIM
        )
        add_custom_command(${build_macos_bundle_})
    endif()

endfunction()

# Deploy bundleable executables on Linux
function(DEPLOY_BUNDLEABLE_LINUX name_ outputDir libraries_ bundle companions opengl_)
#    file(REAL_PATH ${outputDir} outputDir)

    add_custom_command(
        TARGET ${name_}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${outputDir}"
        COMMAND
        "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${name_}>" "${outputDir}"
        || "${CMAKE_COMMAND}" -E remove "$<TARGET_FILE:${name_}>"
        && "${CMAKE_COMMAND}" -E touch_nocreate .notexist
        COMMENT "Copy to destination"
    )
    if (USE_LINUX_APPIMAGES)
        set(build_appimage_
            TARGET ${name_}
            POST_BUILD
            COMMAND test -d "${outputDir}/${name_}.AppDir" && rm -r "${outputDir}/${name_}.AppDir" || true
            COMMAND mkdir -p "${outputDir}/${name_}.AppDir/usr/bin"
            COMMAND mkdir -p "${outputDir}/${name_}.AppDir/usr/share/applications"
            COMMAND mv "${outputDir}/${name_}" "${outputDir}/${name_}.AppDir/usr/bin"
        )
        set(iconname_)
        list(REVERSE companions)
        foreach (companion_ ${companions})
            if (companion_ MATCHES .*\.png)
                list(APPEND build_appimage_
                    COMMAND cp "${companion_}" "${outputDir}/${name_}.AppDir/"
                )
                set(iconname_ ${companion_})
            elseif (companion_ MATCHES .*/Info\.plist)
            	# skip
            else()
                list(APPEND build_appimage_
                    COMMAND cp "${companion_}" "${outputDir}/${name_}.AppDir/"
                )
            endif()
        endforeach()
        if (iconname_)
            get_filename_component(iconname_ ${iconname_} NAME_WE)
        endif()
        list(APPEND build_appimage_
            COMMAND echo "[Desktop Entry]\\nName=${name_}\\nExec=${name_}\\nIcon=${iconname_}\\nType=Application\\nCategories=Utility\;\\n" > "${outputDir}/${name_}.AppDir/usr/share/applications/info.desktop"
        )
        # We use appimagetool (https://github.com/probonopd/go-appimage/releases/tag/continuous)
        # to populate and create the AppImage from the AppDir.
        # This must be downloaded manually by the user.
        execute_process(COMMAND which appimagetool RESULT_VAR r_ OUTPUT_QUIET)
        if (r_)
            MESSAGE(FATAL "Could not locate appimagetool, "
                "make sure to download it from https://github.com/probonopd/go-appimage/releases/tag/continuous "
                "and rename it to appimagetool"
            )
        endif()
        set(selfcontained -s)
        if (opengl_)
            set(selfcontained)
        endif()
        list(APPEND build_appimage_
            COMMAND appimagetool ${selfcontained} deploy "${outputDir}/${name_}.AppDir/usr/share/applications/info.desktop"
            COMMAND cd ${outputDir} && VERSION=1.0 ARCH=x86_64 appimagetool "${outputDir}/${name_}.AppDir"
            COMMAND rm -r "${outputDir}/${name_}.AppDir"
            COMMAND mv "${outputDir}/${name_}-1.0-x86_64.AppImage" "${outputDir}/${name_}"
        )
        list(APPEND build_appimage_
            COMMENT "Deploying ${name_} as a Linux AppImage"
            VERBATIM
        )
        add_custom_command(${build_appimage_})
    endif()

endfunction()

# Create a shortcut to the target executable in the given directory
function(CREATE_SHORTCUT_WIN32 name_ outputDir shortcut_dir_)
  list(GET shortcut_dir_ -1 shortcut_dir_)
  add_custom_command(
    TARGET ${name_}
    POST_BUILD
    COMMAND make_win32_shortcut "${outputDir}/${name_}.exe" "${shortcut_dir_}"
    COMMENT "Creating shortcut"
    VERBATIM
  )
endfunction()

# Create a finder alias to the target app in the given directory
function(CREATE_SHORTCUT_APPLE name_ outputDir shortcut_dir_)
  list(GET shortcut_dir_ -1 shortcut_dir_)
  if (USE_MACOS_APPLICATION_BUNDLES)
    set(target_dir_ ${outputDir}/${name_}.app)
  else()
    set(target_dir_ ${outputDir}/${name_})
  endif()

  add_custom_command(
    TARGET ${name_}
    POST_BUILD
    COMMAND rm "${shortcut_dir_}/${name_}" || true
    COMMAND make_macOS_alias "${target_dir_}" "${shortcut_dir_}/${name_}" || true
    COMMENT "Creating finder alias"
    VERBATIM
  )
endfunction()

# Create a symbolic link to the target app in the given directory
function(CREATE_SHORTCUT_LINUX name_ outputDir shortcut_dir_)
  list(GET shortcut_dir_ -1 shortcut_dir_)
  add_custom_command(
    TARGET ${name_}
    POST_BUILD
    COMMAND ln -s -f "${outputDir}/${name_}" "${shortcut_dir_}/${name_}" || true
    COMMENT "Creating symlink"
    VERBATIM
  )
endfunction()

