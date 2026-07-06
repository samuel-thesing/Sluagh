project(app)

set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROJECT_BINARY_DIR "${WORKSPACE_BINARY_DIR}")
set(PROJECT_SOURCE_DIR "${PROJECT_DIR}/src")
set(PROJECT_RESOURCE_DIR "${PROJECT_DIR}/rsc")

add_executable(${APP_TARGET_NAME} ${APP_SUBSYSTEM}
    src/Entrypoint.cpp
)

target_include_directories(${APP_TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(${APP_TARGET_NAME} PUBLIC
    sluagh::sluagh
    project_warnings
    project_optimizations

    SDL3::SDL3
    SDL3_image::SDL3_image
    spdlog::spdlog
)

target_compile_definitions(${APP_TARGET_NAME} PRIVATE
    ${APP_DEFINES}
)

set_target_properties(${APP_TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}

    OUTPUT_NAME ${APP_EXECUTABLE_NAME}
)

# copy DLLs
if (WIN32 AND BUILD_SHARED)
    add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${APP_TARGET_NAME}>
        $<TARGET_FILE_DIR:${APP_TARGET_NAME}>
        COMMAND_EXPAND_LISTS
    )
elseif (UNIX AND BUILD_SHARED)
    add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:${APP_TARGET_NAME}>
    )
endif ()

copy_resources(
    TARGET ${APP_TARGET_NAME}
    TARGET_DIR "${PROJECT_BINARY_DIR}/rsc"
    RESOURCE_DIR "${WORKSPACE_RESOURCE_DIR}"
    EXCLUDE "${WORKSPACE_RESOURCE_DIR}/shader"
    ONCE
)
copy_resources(
    TARGET ${APP_TARGET_NAME}
    TARGET_DIR "${PROJECT_BINARY_DIR}/rsc"
    RESOURCE_DIR "${PROJECT_RESOURCE_DIR}"
    EXCLUDE "${PROJECT_RESOURCE_DIR}/shader"
    ONCE
)

compile_shaders(
    TARGET ${APP_TARGET_NAME}
    TARGET_DIR "${PROJECT_BINARY_DIR}/rsc/shader"
    SHADER_DIR "${WORKSPACE_RESOURCE_DIR}/shader"
    ONCE
)
compile_shaders(
    TARGET ${APP_TARGET_NAME}
    TARGET_DIR "${PROJECT_BINARY_DIR}/rsc/shader"
    SHADER_DIR "${PROJECT_RESOURCE_DIR}/shader"
    ONCE
)