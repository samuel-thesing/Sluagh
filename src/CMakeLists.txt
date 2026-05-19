project(sluagh)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_BINARY_DIR "${WORKSPACE_BINARY_DIR}")
set(PROJECT_ARTIFACT_DIR "${WORKSPACE_ARTIFACT_DIR}")
set(PROJECT_SOURCE_DIR "${PROJECT_DIR}")

if (BUILD_SHARED)
    set(_lib_type SHARED)
else ()
    set(_lib_type STATIC)
endif ()

add_library(${PROJECT_NAME} ${_lib_type}
    sluagh.cpp
    core/Application.cpp
    render/Renderer.cpp
    render/Renderer2d.cpp
    render/utils.cpp
    utils/Color.cpp
    utils/Logger.cpp
    utils/Arguments.cpp
)

add_library(sluagh::sluagh ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>

    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} # don't expose sources and internal headers
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    project_warnings
    project_optimizations

    SDL3::SDL3
    spdlog::spdlog
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME ${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}/export.h
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SLUAGH_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
    SLUAGH_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
    SLUAGH_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH}

    SLUAGH_EXPORTS
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_ARTIFACT_DIR}

    # default to no-export on linux
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)