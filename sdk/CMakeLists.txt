cmake_minimum_required(VERSION 3.25)
project(RivetSDK)

# --- Fetching Vendors --- #
include(FetchContent)

message(STATUS "Fetching raylib...")
# raylib's own CMake only has the desktop backends (pointed at iOS, it still picks desktop OpenGL).
# On iOS, SOURCE_SUBDIR names a folder without a CMakeLists.txt, so the sources are fetched but
# raylib's project isn't added, and the library is built with the iOS backend below instead.
if(IOS)
    set(RAYLIB_FETCH_OPTIONS SOURCE_SUBDIR "projects/Xcode27")
endif()
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/ghera/raylib-iOS.git"
    GIT_TAG "6.2.0-iOS"
    ${RAYLIB_FETCH_OPTIONS}
)
FetchContent_MakeAvailable(raylib)

if(IOS)
    # Linking ANGLE's .xcframeworks needs CMake 3.28, and embedding and signing them in the app
    # needs the Xcode generator (the ios-* presets use it).
    if(CMAKE_VERSION VERSION_LESS 3.28 OR NOT CMAKE_GENERATOR STREQUAL "Xcode")
        message(FATAL_ERROR "iOS builds need CMake 3.28 or newer and the Xcode generator.")
    endif()

    # The same sources and settings as the fork's projects/Xcode27 project.
    set(RAYLIB_SRC ${raylib_SOURCE_DIR}/src)
    add_library(raylib STATIC
        ${RAYLIB_SRC}/rcore.c
        ${RAYLIB_SRC}/rshapes.c
        ${RAYLIB_SRC}/rtextures.c
        ${RAYLIB_SRC}/rtext.c
        ${RAYLIB_SRC}/rmodels.c
        ${RAYLIB_SRC}/raudio.c
    )

    # rcore.c includes the UIKit backend (platforms/rcore_ios.c), and raudio.c's miniaudio drives
    # AVAudioSession, so both are compiled as Objective-C.
    set_source_files_properties(${RAYLIB_SRC}/rcore.c ${RAYLIB_SRC}/raudio.c
        PROPERTIES
            LANGUAGE OBJC
    )

    # The backend relies on ARC. rlgl.h includes ANGLE's GLES/glext.h before the header that
    # defines its types, which only compiles when the include imports the framework as a module.
    set_target_properties(raylib PROPERTIES
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
        XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES YES
    )

    target_include_directories(raylib
        PUBLIC
            ${RAYLIB_SRC}
    )

    # PUBLIC like the platform and graphics defines of raylib's own CMake, so that
    # Rivet/EntryPoint.hpp can tell which platform it's on.
    target_compile_definitions(raylib
        PUBLIC
            PLATFORM_IOS
            GRAPHICS_API_OPENGL_ES3
        PRIVATE
            GL_GLEXT_PROTOTYPES
    )

    # OpenGL ES comes from ANGLE, which runs it on Metal; the fork ships it prebuilt. Linking the
    # .xcframeworks lets Xcode pick the device or simulator slice, and rivet_configure_app()
    # embeds them in the app.
    target_link_libraries(raylib
        PUBLIC
            ${raylib_SOURCE_DIR}/deps/ANGLE/libEGL.xcframework
            ${raylib_SOURCE_DIR}/deps/ANGLE/libGLESv2.xcframework
            "-framework UIKit"
            "-framework QuartzCore"
            "-framework CoreGraphics"
            "-framework Foundation"
            "-framework CoreFoundation"
            "-framework AVFoundation"
            "-framework AudioToolbox"
    )
endif()

message(STATUS "Fetching physfs...")
set(PHYSFS_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(PHYSFS_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    physfs
    GIT_REPOSITORY "https://github.com/icculus/physfs.git"
    GIT_TAG "main"
)
FetchContent_MakeAvailable(physfs)

message(STATUS "Fetching box2d")
set(BOX3D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX3D_TESTS OFF CACHE BOOL "" FORCE)
set(BOX3D_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    box2d
    GIT_REPOSITORY "https://github.com/erincatto/box2d.git"
    GIT_TAG "v3.1.1"
)
FetchContent_MakeAvailable(box2d)

message(STATUS "Fetching box3d")
set(BOX3D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX3D_TESTS OFF CACHE BOOL "" FORCE)
set(BOX3D_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    box3d
    GIT_REPOSITORY "https://github.com/erincatto/box3d.git"
    GIT_TAG "v0.1.0"
)
FetchContent_MakeAvailable(box3d)

message(STATUS "Fetching tomlplusplus...")
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY "https://github.com/marzer/tomlplusplus.git"
    GIT_TAG "v3.4.0"
)
FetchContent_MakeAvailable(tomlplusplus)

message(STATUS "Fetching sol2")
FetchContent_Declare(
    sol2
    GIT_REPOSITORY "https://github.com/ThePhD/sol2.git"
    GIT_TAG "v3.5.0"
)
FetchContent_MakeAvailable(sol2)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

# Have sol2 check what scripts pass in, so calling a method with '.' rather than ':' raises a Lua
# error instead of crashing. PUBLIC, since everything that includes sol2 has to agree on this.
target_compile_definitions(${PROJECT_NAME} PUBLIC SOL_ALL_SAFETIES_ON=1)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        raylib
        PhysFS::PhysFS-static
        box3d
        box2d
        lua
        imgui
        sol2
        tomlplusplus::tomlplusplus
)

# --- App Packaging --- #
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/RivetApp.cmake)