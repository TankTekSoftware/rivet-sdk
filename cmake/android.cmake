# Finds the Android SDK and the NDK inside it, picks the ABI to build for, and hands the rest to the
# NDK's own toolchain file. Each build tree builds for one ABI.

# The SDK is where Android Studio installs it, unless ANDROID_HOME (or the older ANDROID_SDK_ROOT)
# says otherwise. rivet_configure_app() also takes the tools that package APKs from it.
if(NOT RIVET_ANDROID_SDK)
    if(DEFINED ENV{ANDROID_HOME})
        set(_rivet_sdk "$ENV{ANDROID_HOME}")
    elseif(DEFINED ENV{ANDROID_SDK_ROOT})
        set(_rivet_sdk "$ENV{ANDROID_SDK_ROOT}")
    elseif(CMAKE_HOST_WIN32)
        set(_rivet_sdk "$ENV{LOCALAPPDATA}/Android/Sdk")
    elseif(CMAKE_HOST_APPLE)
        set(_rivet_sdk "$ENV{HOME}/Library/Android/sdk")
    else()
        set(_rivet_sdk "$ENV{HOME}/Android/Sdk")
    endif()
    file(TO_CMAKE_PATH "${_rivet_sdk}" _rivet_sdk)

    if(NOT IS_DIRECTORY "${_rivet_sdk}")
        message(FATAL_ERROR
            "Found no Android SDK at ${_rivet_sdk}. Install Android Studio, or set ANDROID_HOME to "
            "the folder that holds the SDK."
        )
    endif()

    set(RIVET_ANDROID_SDK "${_rivet_sdk}" CACHE PATH "Android SDK")
    unset(_rivet_sdk)
endif()

# The NDK named by ANDROID_NDK_HOME (or ANDROID_NDK_ROOT), or else the newest of those the SDK
# Manager installed side by side.
if(NOT RIVET_ANDROID_NDK)
    if(DEFINED ENV{ANDROID_NDK_HOME})
        set(_rivet_ndks "$ENV{ANDROID_NDK_HOME}")
    elseif(DEFINED ENV{ANDROID_NDK_ROOT})
        set(_rivet_ndks "$ENV{ANDROID_NDK_ROOT}")
    else()
        file(GLOB _rivet_ndks LIST_DIRECTORIES true "${RIVET_ANDROID_SDK}/ndk/*")
        list(SORT _rivet_ndks COMPARE NATURAL ORDER DESCENDING)
    endif()

    foreach(_rivet_ndk IN LISTS _rivet_ndks)
        file(TO_CMAKE_PATH "${_rivet_ndk}" _rivet_ndk)
        if(EXISTS "${_rivet_ndk}/build/cmake/android.toolchain.cmake")
            set(RIVET_ANDROID_NDK "${_rivet_ndk}" CACHE PATH "Android NDK")
            break()
        endif()
    endforeach()
    unset(_rivet_ndk)
    unset(_rivet_ndks)

    if(NOT RIVET_ANDROID_NDK)
        message(FATAL_ERROR
            "Found no Android NDK in ${RIVET_ANDROID_SDK}/ndk. Install one with Android Studio's SDK "
            "Manager (SDK Tools > NDK (Side by side)), or set ANDROID_NDK_HOME to one."
        )
    endif()
endif()

# Devices are arm64. Emulators run fastest with the architecture of the computer they run on,
# which is arm64 on Apple silicon and x86_64 on most PCs.
option(RIVET_ANDROID_EMULATOR "Build for the Android Emulator on this computer, rather than for devices" OFF)
if(NOT ANDROID_ABI)
    string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" _rivet_host_processor)
    if(RIVET_ANDROID_EMULATOR AND NOT _rivet_host_processor MATCHES "^(arm64|aarch64)$")
        set(ANDROID_ABI "x86_64" CACHE STRING "Android ABI to build for")
    else()
        set(ANDROID_ABI "arm64-v8a" CACHE STRING "Android ABI to build for")
    endif()
    unset(_rivet_host_processor)
endif()

# The oldest Android that games run on: 7.0.
set(ANDROID_PLATFORM "android-24" CACHE STRING "Oldest Android API level that games run on")

include("${RIVET_ANDROID_NDK}/build/cmake/android.toolchain.cmake")

# try_compile() runs this file again in a project of its own, which only gets the variables listed
# here. The NDK's toolchain sets the list, so these go on after it.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES RIVET_ANDROID_SDK RIVET_ANDROID_NDK)
