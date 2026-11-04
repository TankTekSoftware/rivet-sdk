cmake_minimum_required(VERSION 3.25)
project(RivetRuntime)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# The runtime is a game as far as the build is concerned, so it's added the way one is: on Android
# it has to be the shared library that NativeActivity loads, not an executable.
rivet_add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        RivetSDK
)
