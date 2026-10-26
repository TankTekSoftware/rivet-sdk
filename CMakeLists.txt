cmake_minimum_required(VERSION 3.25)
project(Rivet)

# --- Compile Commands Settings --- #
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(COMPILE_COMMANDS_TOP ${CMAKE_SOURCE_DIR}/compile_commands.json)

if(NOT TARGET export_compile_commands_top)
    add_custom_target(export_compile_commands_top ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${COMPILE_COMMANDS_TOP}
        COMMENT "Exporting compile_commands.json to top level"
        VERBATIM
    )
endif()

# --- Configure C++ --- #
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# raylib's GLFW and PhysFS both compile Objective-C sources on macOS. GLFW only enables OBJC in its
# own directory, so PhysFS's .m file would have no build rules; enabling it here covers every project.
if(APPLE)
    enable_language(OBJC)
endif()

# --- Configure Output Directory --- #
string(TOLOWER "${PLATFORM}" PLATFORM_DIR)
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_DIR}-${BUILD_TYPE})

# Adds a project that builds into bin/<platform>-<build type>/<name>/, so the content deployed
# next to one executable can't overwrite another's. Vendored libraries build into the folder of
# the project that fetches them.
function(add_project directory name)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}/${name})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR}/${name})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}/${name})
    add_subdirectory(${directory})
endfunction()

# --- Compile Projects --- #
add_project(${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui imgui)
add_project(${CMAKE_CURRENT_SOURCE_DIR}/vendor/lua lua)

add_project(${CMAKE_CURRENT_SOURCE_DIR}/sdk/ rivetsdk)

add_project(${CMAKE_CURRENT_SOURCE_DIR}/examples/pong/ pong)