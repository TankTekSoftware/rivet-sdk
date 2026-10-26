set(CMAKE_SYSTEM_NAME iOS)

# The deployment target of the raylib-iOS fork's own Xcode project.
set(CMAKE_OSX_DEPLOYMENT_TARGET "15.6" CACHE STRING "Minimum iOS version")

# Without a run destination, xcodebuild builds the simulator for every architecture. Building just
# this Mac's keeps command-line simulator builds from compiling everything twice.
if(CMAKE_OSX_SYSROOT MATCHES "[Ss]imulator")
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "")
endif()

# Generate one Xcode project for the whole tree, rather than one for every project() call.
set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON)
