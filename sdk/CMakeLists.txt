cmake_minimum_required(VERSION 3.25)
project(RivetSDK)

# --- Fetching Vendors --- #
include(FetchContent)

message(STATUS "Fetching raylib...")
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/ghera/raylib-iOS.git"
    GIT_TAG "6.2.0-iOS"
)
FetchContent_MakeAvailable(raylib)

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