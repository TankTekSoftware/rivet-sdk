# The Apple development team that signs iOS device builds. The environment variable of the same
# name wins when it's set; otherwise the build tree keeps the last value, so it's only given once.
if(IOS)
    set(RIVET_IOS_DEVELOPMENT_TEAM "" CACHE STRING "Apple development team ID that signs iOS device builds")
    if(DEFINED ENV{RIVET_IOS_DEVELOPMENT_TEAM})
        set_property(CACHE RIVET_IOS_DEVELOPMENT_TEAM PROPERTY VALUE "$ENV{RIVET_IOS_DEVELOPMENT_TEAM}")
    endif()
endif()

# rivet_configure_app(<target> BUNDLE_ID <id> [DISPLAY_NAME <name>])
#
# Packages a game executable the way its platform runs it. On iOS that is an app bundle, with an
# Info.plist, the frameworks raylib renders through and code signing; simulator builds also get a
# run_<target> target that starts the app in the iOS Simulator. Desktop platforms run the
# executable as built.
function(rivet_configure_app target)
    cmake_parse_arguments(PARSE_ARGV 1 APP "" "BUNDLE_ID;DISPLAY_NAME" "")

    if(NOT IOS)
        return()
    endif()

    if(NOT APP_BUNDLE_ID)
        message(FATAL_ERROR "rivet_configure_app(${target}): iOS apps need a BUNDLE_ID.")
    endif()

    if(NOT APP_DISPLAY_NAME)
        set(APP_DISPLAY_NAME ${target})
    endif()

    # The version comes from the game's project() call, when it gives one.
    set(APP_VERSION 1.0)
    if(PROJECT_VERSION)
        set(APP_VERSION ${PROJECT_VERSION})
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
