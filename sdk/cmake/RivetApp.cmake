# The Apple development team that signs iOS device builds. The environment variable of the same
# name wins when it's set; otherwise the build tree keeps the last value, so it's only given once.
if(IOS)
    set(RIVET_IOS_DEVELOPMENT_TEAM "" CACHE STRING "Apple development team ID that signs iOS device builds")
    if(DEFINED ENV{RIVET_IOS_DEVELOPMENT_TEAM})
        set_property(CACHE RIVET_IOS_DEVELOPMENT_TEAM PROPERTY VALUE "$ENV{RIVET_IOS_DEVELOPMENT_TEAM}")
    endif()
endif()

# Finds what packaging Android apps takes, in the SDK that cmake/android.cmake found: its build
# tools, which package and sign APKs, and a platform to build them against. A JDK runs apksigner,
# adds the game's library to the APK and makes the debug key. The build tree keeps what it finds.
#
# Everything is kept in the cache, since rivet_configure_app() runs in the games' directories,
# which can't see the variables of this one.
function(_rivet_find_android_tools)
    # The newest build tools. Version 35 is the first whose zipalign aligns libraries for the
    # 16 KB memory pages that devices can have since Android 15.
    if(NOT RIVET_ANDROID_BUILD_TOOLS)
        file(GLOB dirs LIST_DIRECTORIES true "${RIVET_ANDROID_SDK}/build-tools/*")
        list(SORT dirs COMPARE NATURAL ORDER DESCENDING)
        foreach(dir IN LISTS dirs)
            get_filename_component(version "${dir}" NAME)
            if(version VERSION_GREATER_EQUAL 35 AND EXISTS "${dir}/lib/apksigner.jar")
                set(RIVET_ANDROID_BUILD_TOOLS "${dir}" CACHE PATH "Android SDK build tools that package APKs")
                break()
            endif()
        endforeach()

        if(NOT RIVET_ANDROID_BUILD_TOOLS)
            message(FATAL_ERROR
                "Found no Android SDK build tools of version 35 or newer in "
                "${RIVET_ANDROID_SDK}/build-tools. Install them with Android Studio's SDK Manager "
                "(SDK Tools > Android SDK Build-Tools)."
            )
        endif()
    endif()

    # The newest platform. Previews are named after a codename rather than an API level.
    if(NOT RIVET_ANDROID_PLATFORM)
        file(GLOB dirs LIST_DIRECTORIES true "${RIVET_ANDROID_SDK}/platforms/android-*")
        list(SORT dirs COMPARE NATURAL ORDER DESCENDING)
        foreach(dir IN LISTS dirs)
            get_filename_component(name "${dir}" NAME)
            if(name MATCHES "^android-[0-9]+" AND EXISTS "${dir}/android.jar")
                set(RIVET_ANDROID_PLATFORM "${dir}" CACHE PATH "Android SDK platform that APKs are built against")
                break()
            endif()
        endforeach()

        if(NOT RIVET_ANDROID_PLATFORM)
            message(FATAL_ERROR
                "Found no Android SDK platform in ${RIVET_ANDROID_SDK}/platforms. Install one with "
                "Android Studio's SDK Manager (SDK Platforms)."
            )
        endif()
    endif()

    # Apps target the platform they're built against, unless they're told otherwise.
    if(NOT RIVET_ANDROID_TARGET_SDK)
        get_filename_component(name "${RIVET_ANDROID_PLATFORM}" NAME)
        string(REGEX MATCH "[0-9]+" level "${name}")
        set(RIVET_ANDROID_TARGET_SDK "${level}" CACHE STRING "Android API level that apps target")
    endif()

    # A JDK: the one JAVA_HOME names, else one on the PATH, else the one inside Android Studio.
    if(CMAKE_HOST_WIN32)
        set(studio_jdk "$ENV{ProgramFiles}/Android/Android Studio/jbr")
    elseif(CMAKE_HOST_APPLE)
        set(studio_jdk "/Applications/Android Studio.app/Contents/jbr/Contents/Home")
    else()
        set(studio_jdk "/opt/android-studio/jbr")
    endif()

    foreach(tool java jar keytool)
        string(TOUPPER "${tool}" name)
        find_program(RIVET_JDK_${name} ${tool}
            HINTS ENV JAVA_HOME
            PATHS "${studio_jdk}"
            PATH_SUFFIXES bin
            NO_CMAKE_FIND_ROOT_PATH
        )
        if(NOT RIVET_JDK_${name})
            message(FATAL_ERROR
                "Found no ${tool}, which packaging Android apps needs. Set JAVA_HOME to a JDK (17 or "
                "newer), or install Android Studio, which comes with one."
            )
        endif()
    endforeach()

    # Since Java 24, apksigner loading its native crypto library prints warnings unless Java is
    # told to allow it, with an option that older versions might not take.
    if(NOT DEFINED RIVET_JDK_VERSION)
        execute_process(COMMAND "${RIVET_JDK_JAVA}" -version ERROR_VARIABLE version OUTPUT_QUIET)
        string(REGEX MATCH "version \"([0-9]+)" version "${version}")
        set(RIVET_JDK_VERSION "${CMAKE_MATCH_1}" CACHE INTERNAL "Major version of RIVET_JDK_JAVA")
    endif()
endfunction()

if(ANDROID)
    _rivet_find_android_tools()

    # The keystore that signs Android apps, and which key in it does (only needed when it holds
    # more than one). As with the iOS team, the environment variables of the same names win, and
    # otherwise the build tree keeps the last values. Without a keystore, the debug key signs.
    set(RIVET_ANDROID_KEYSTORE "" CACHE FILEPATH "Keystore that signs Android apps (the debug key signs them without one)")
    set(RIVET_ANDROID_KEY_ALIAS "" CACHE STRING "Key in RIVET_ANDROID_KEYSTORE that signs Android apps")
    foreach(variable RIVET_ANDROID_KEYSTORE RIVET_ANDROID_KEY_ALIAS)
        if(DEFINED ENV{${variable}})
            set_property(CACHE ${variable} PROPERTY VALUE "$ENV{${variable}}")
        endif()
    endforeach()
endif()

# Gives the path of the debug key that Android Studio and Gradle sign debug builds with, making it
# the way they do when it isn't there yet.
function(_rivet_android_debug_keystore out_var)
    if(DEFINED ENV{ANDROID_USER_HOME})
        set(dir "$ENV{ANDROID_USER_HOME}")
    elseif(CMAKE_HOST_WIN32)
        set(dir "$ENV{USERPROFILE}/.android")
    else()
        set(dir "$ENV{HOME}/.android")
    endif()
    file(TO_CMAKE_PATH "${dir}" dir)
    set(keystore "${dir}/debug.keystore")

    if(NOT EXISTS "${keystore}")
        file(MAKE_DIRECTORY "${dir}")
        execute_process(
            COMMAND "${RIVET_JDK_KEYTOOL}" -genkeypair
                -keystore "${keystore}" -storepass android
                -alias androiddebugkey -keypass android
                -keyalg RSA -keysize 2048 -validity 10000
                -dname "CN=Android Debug,O=Android,C=US"
            RESULT_VARIABLE result
            OUTPUT_VARIABLE output
            ERROR_VARIABLE output
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Couldn't make the Android debug key at ${keystore}:\n${output}")
        endif()
        message(STATUS "Made the Android debug key, at ${keystore}")
    endif()

    set(${out_var} "${keystore}" PARENT_SCOPE)
endfunction()

# rivet_add_executable(<target> [<source>...])
#
# Adds a game's executable, in place of add_executable(). Android doesn't start native executables:
# its NativeActivity loads the game as a shared library instead, so that is what this builds on
# Android (lib<target>.so). Everywhere else it's add_executable().
function(rivet_add_executable target)
    if(ANDROID)
        add_library(${target} SHARED ${ARGN})
    else()
        add_executable(${target} ${ARGN})
    endif()
endfunction()

# rivet_configure_app(<target> BUNDLE_ID <id> [DISPLAY_NAME <name>] [ORIENTATION <portrait|landscape>])
#
# Packages a game executable the way its platform runs it. On iOS that is an app bundle, with an
# Info.plist, the frameworks raylib renders through and code signing; simulator builds also get a
# run_<target> target that starts the app in the iOS Simulator. On Android it is a signed APK
# (<target>.apk, next to the game's library) that starts the game in a NativeActivity, and every
# build gets a run_<target> target that installs and starts it on a device or emulator. raylib's
# Android backend can't follow the screen as it rotates, so Android holds the game in ORIENTATION
# (landscape unless told otherwise); iOS rotates freely. Desktop platforms run the executable as
# built.
function(rivet_configure_app target)
    cmake_parse_arguments(PARSE_ARGV 1 APP "" "BUNDLE_ID;DISPLAY_NAME;ORIENTATION" "")

    if(NOT IOS AND NOT ANDROID)
        return()
    endif()

    if(NOT APP_BUNDLE_ID)
        message(FATAL_ERROR "rivet_configure_app(${target}): iOS and Android apps need a BUNDLE_ID.")
    endif()

    if(NOT APP_DISPLAY_NAME)
        set(APP_DISPLAY_NAME ${target})
    endif()

    # The version comes from the game's project() call, when it gives one.
    set(APP_VERSION 1.0)
    if(PROJECT_VERSION)
        set(APP_VERSION ${PROJECT_VERSION})
    endif()

    if(ANDROID)
        _rivet_configure_android_app(${target})
        return()
    endif()

    # raylib links ANGLE dynamically, so its frameworks have to ship inside the app.
    get_target_property(EMBEDDED_FRAMEWORKS raylib INTERFACE_LINK_LIBRARIES)
    list(FILTER EMBEDDED_FRAMEWORKS INCLUDE REGEX "\\.xcframework$")

    set_target_properties(${target} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ios/Info.plist.in
        MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_BUNDLE_ID}
        MACOSX_BUNDLE_BUNDLE_NAME ${APP_DISPLAY_NAME}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION}
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${APP_BUNDLE_ID}
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" # iPhone and iPad
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks"
        XCODE_EMBED_FRAMEWORKS "${EMBEDDED_FRAMEWORKS}"
        XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
        XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY ON
        # A scheme, so the game is ready to run when the project is opened in Xcode.
        XCODE_GENERATE_SCHEME ON
    )

    # iOS only installs device builds signed by a development team; Xcode manages the certificate
    # and provisioning profile for it. Simulator builds are signed locally and need no team.
    if(RIVET_IOS_DEVELOPMENT_TEAM)
        set_property(TARGET ${target} PROPERTY XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${RIVET_IOS_DEVELOPMENT_TEAM})
    elseif(NOT CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM AND NOT CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
        message(WARNING
            "${target} can't be signed for devices without a development team. Configure again with "
            "RIVET_IOS_DEVELOPMENT_TEAM set to your team ID (Xcode > Settings > Accounts), either "
            "as an environment variable or with -DRIVET_IOS_DEVELOPMENT_TEAM=<team ID>."
        )
    endif()

    if(CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
        # Not VERBATIM: under Xcode the bundle path can hold ${EFFECTIVE_PLATFORM_NAME}, which the
        # shell has to fill in from the build's environment.
        add_custom_target(run_${target}
            COMMAND /bin/sh ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ios/run-simulator.sh $<TARGET_BUNDLE_DIR:${target}>
            COMMENT "Running ${target} in the iOS Simulator"
        )
        add_dependencies(run_${target} ${target})
    endif()
endfunction()

# The Android half of rivet_configure_app(), which calls it with its APP_ values in scope.
function(_rivet_configure_android_app target)
    # The sensor orientations still let the game be turned upside down.
    if(NOT APP_ORIENTATION)
        set(APP_ORIENTATION landscape)
    endif()
    if(APP_ORIENTATION STREQUAL "portrait")
        set(APP_SCREEN_ORIENTATION sensorPortrait)
    elseif(APP_ORIENTATION STREQUAL "landscape")
        set(APP_SCREEN_ORIENTATION sensorLandscape)
    else()
        message(FATAL_ERROR
            "rivet_configure_app(${target}): ORIENTATION is portrait or landscape, not \"${APP_ORIENTATION}\"."
        )
    endif()

    # Android tells an app's versions apart by a number that has to grow with each one, so the
    # version becomes one: 1.2.3 is 10203.
    string(REGEX MATCHALL "[0-9]+" version_parts "${APP_VERSION}.0.0")
    list(GET version_parts 0 major)
    list(GET version_parts 1 minor)
    list(GET version_parts 2 patch)
    math(EXPR APP_VERSION_CODE "${major} * 10000 + ${minor} * 100 + ${patch}")

    set(APP_MIN_SDK ${ANDROID_PLATFORM_LEVEL})
    set(APP_TARGET_SDK ${RIVET_ANDROID_TARGET_SDK})
    # NativeActivity loads lib<APP_LIB_NAME>.so. The name is only known at generate time, which is
    # why the manifest is written by file(GENERATE).
    set(APP_LIB_NAME "$<TARGET_FILE_BASE_NAME:${target}>")

    set(work "${CMAKE_CURRENT_BINARY_DIR}/${target}-apk")
    file(READ "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/android/AndroidManifest.xml.in" manifest)
    string(CONFIGURE "${manifest}" manifest @ONLY)
    file(GENERATE OUTPUT "${work}/AndroidManifest.xml" CONTENT "${manifest}")

    if(RIVET_ANDROID_KEYSTORE)
        # apksigner reads the password from the environment as it signs, so it's never stored.
        set(signing --ks "${RIVET_ANDROID_KEYSTORE}" --ks-pass env:RIVET_ANDROID_KEYSTORE_PASSWORD)
        if(RIVET_ANDROID_KEY_ALIAS)
            list(APPEND signing --ks-key-alias "${RIVET_ANDROID_KEY_ALIAS}")
        endif()
    else()
        _rivet_android_debug_keystore(debug_keystore)
        set(signing --ks "${debug_keystore}" --ks-pass pass:android --ks-key-alias androiddebugkey)
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS
                "${target}.apk is signed with the debug key, which only suits testing. Set "
                "RIVET_ANDROID_KEYSTORE to sign it for release."
            )
        endif()
    endif()

    set(apksigner_java_options "")
    if(RIVET_JDK_VERSION GREATER_EQUAL 24)
        set(apksigner_java_options --enable-native-access=ALL-UNNAMED)
    endif()

    # The APK goes next to the library, in bin/.
    get_target_property(output_dir ${target} LIBRARY_OUTPUT_DIRECTORY)
    if(NOT output_dir)
        set(output_dir "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    set(apk "${output_dir}/${target}.apk")
    set(lib "lib/${ANDROID_ABI}/$<TARGET_FILE_NAME:${target}>")

    add_custom_command(
        OUTPUT "${apk}"
        # The packaged library goes without its debug info, and in release builds its symbol table
        # too. Debug builds keep the symbols so crashes in the device log name their functions,
        # and the library in bin/ keeps everything for debuggers.
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${work}/lib/${ANDROID_ABI}"
        COMMAND "${CMAKE_STRIP}" "$<IF:$<CONFIG:Debug>,--strip-debug,--strip-unneeded>"
            -o "${work}/${lib}" "$<TARGET_FILE:${target}>"
        # The compiled manifest, which debug builds mark as debuggable.
        COMMAND "${RIVET_ANDROID_BUILD_TOOLS}/aapt2${CMAKE_HOST_EXECUTABLE_SUFFIX}" link
            -o "${work}/unaligned.apk"
            -I "${RIVET_ANDROID_PLATFORM}/android.jar"
            --manifest "${work}/AndroidManifest.xml"
            "$<$<CONFIG:Debug>:--debug-mode>"
        # The library is stored uncompressed, so Android can load it straight from the APK. That
        # takes it aligned to memory pages, and the whole APK has to be signed once it's final.
        COMMAND "${RIVET_JDK_JAR}" --update --no-manifest --no-compress
            --file "${work}/unaligned.apk" -C "${work}" "${lib}"
        COMMAND "${RIVET_ANDROID_BUILD_TOOLS}/zipalign${CMAKE_HOST_EXECUTABLE_SUFFIX}" -P 16 -f 4
            "${work}/unaligned.apk" "${work}/aligned.apk"
        COMMAND "${RIVET_JDK_JAVA}" ${apksigner_java_options}
            -jar "${RIVET_ANDROID_BUILD_TOOLS}/lib/apksigner.jar" sign
            ${signing} --out "${apk}" "${work}/aligned.apk"
        DEPENDS ${target} "${work}/AndroidManifest.xml"
        COMMENT "Packaging ${target}.apk"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
    add_custom_target(apk_${target} ALL DEPENDS "${apk}")

    add_custom_target(run_${target}
        COMMAND "${CMAKE_COMMAND}"
            "-DADB=${RIVET_ANDROID_SDK}/platform-tools/adb${CMAKE_HOST_EXECUTABLE_SUFFIX}"
            "-DEMULATOR=${RIVET_ANDROID_SDK}/emulator/emulator${CMAKE_HOST_EXECUTABLE_SUFFIX}"
            "-DAPK=${apk}"
            "-DPACKAGE=${APP_BUNDLE_ID}"
            "-DUSE_EMULATOR=${RIVET_ANDROID_EMULATOR}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/android/run-device.cmake"
        COMMENT "Running ${target} on Android"
        USES_TERMINAL
        VERBATIM
    )
    add_dependencies(run_${target} apk_${target})
endfunction()
