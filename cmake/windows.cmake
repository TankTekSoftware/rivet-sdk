if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(MINGW_TRIPLET "x86_64-w64-mingw32")

    find_program(MINGW_C_COMPILER "${MINGW_TRIPLET}-gcc")
    find_program(MINGW_CXX_COMPILER "${MINGW_TRIPLET}-g++")
    find_program(MINGW_RC_COMPILER "${MINGW_TRIPLET}-windres")

    if(NOT MINGW_C_COMPILER OR NOT MINGW_CXX_COMPILER OR NOT MINGW_RC_COMPILER)
        message(FATAL_ERROR
            "MinGW-w64 not found. On Arch, install it with: sudo pacman -S mingw-w64-gcc")
    endif()

    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)

    set(CMAKE_C_COMPILER "${MINGW_C_COMPILER}")
    set(CMAKE_CXX_COMPILER "${MINGW_CXX_COMPILER}")
    set(CMAKE_RC_COMPILER "${MINGW_RC_COMPILER}")

    # Search for libraries and headers in the MinGW sysroot, not the Linux system.
    # Build tools still come from the host because they need to run on Linux.
    set(CMAKE_FIND_ROOT_PATH "/usr/${MINGW_TRIPLET}")
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
elseif(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    message(FATAL_ERROR "windows.cmake supports Linux and Windows hosts, not ${CMAKE_HOST_SYSTEM_NAME}.")
endif()