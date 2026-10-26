cmake_minimum_required(VERSION 3.25)
project(Pong)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        RivetSDK
)

rivet_configure_app(${PROJECT_NAME}
    BUNDLE_ID "com.tanktek.pong"
    DISPLAY_NAME "Rivet Pong"
)