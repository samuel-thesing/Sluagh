cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(sluagh
    VERSION 1.0.0
    DESCRIPTION "Game Engine for 2d & 3d using SDL3"
    LANGUAGES CXX
)

set(WORKSPACE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WORKSPACE_BINARY_DIR "${WORKSPACE_DIR}/bin/${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}")
set(WORKSPACE_ARTIFACT_DIR "${WORKSPACE_DIR}/bin-int/${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}")

# prevent installation to system directory
set(CMAKE_INSTALL_PREFIX "${WORKSPACE_BINARY_DIR}" CACHE INTERNAL "")

option(BUILD_SHARED "Build lib as shared" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Options)
include(Dependencies)

add_subdirectory(src)

# windows can't correctly reattach the console if the subsystem is "WINDOWS" (exits early in console)
# => build second executable on windows with subsystem console
if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(APP_TARGET_NAME "app_gui")
    set(APP_SUBSYSTEM "WIN32")
    set(APP_EXECUTABLE_NAME "app")
    set(APP_DEFINES "APP_GUI")
    add_subdirectory(app app-gui)

    set(APP_TARGET_NAME "app_console")
    set(APP_SUBSYSTEM "")
    set(APP_EXECUTABLE_NAME "app_console")
    set(APP_DEFINES "APP_CONSOLE")
    add_subdirectory(app app-console)

    add_custom_target(app DEPENDS app_gui app_console)

else ()
    set(APP_TARGET_NAME "app")
    set(APP_SUBSYSTEM "")
    set(APP_EXECUTABLE_NAME "app")
    set(APP_DEFINES "APP_CONSOLE")
    add_subdirectory(app)
endif ()