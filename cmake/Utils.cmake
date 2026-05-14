function(escape_regex INPUT OUTPUT)
    set(RESULT "${INPUT}")
    foreach(CHAR "\\" "." "+" "*" "?" "^" "$" "{" "}" "[" "]" "(" ")" "|")
        string(REPLACE "${CHAR}" "\\${CHAR}" RESULT "${RESULT}")
    endforeach()
    set(${OUTPUT} "${RESULT}" PARENT_SCOPE)
endfunction()

function(copy_resources)
    set(options "ONCE")
    set(oneValueArgs TARGET TARGET_DIR RESOURCE_DIR)
    set(multiValueArgs FILE_EXTENSIONS EXCLUDE)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    set(COPY_PAIR "${arg_RESOURCE_DIR},${arg_TARGET_DIR}")
    get_property(LOCAL_COPIED_RESOURCES GLOBAL PROPERTY COPIED_RESOURCES)
    list(FIND LOCAL_COPIED_RESOURCES "${COPY_PAIR}" COPY_PAIR_IDX)
    if (${COPY_PAIR_IDX} GREATER_EQUAL 0)
        if (NOT arg_ONCE)
            message(WARNING "Tried to copy from '${arg_RESOURCE_DIR}' to '${arg_TARGET_DIR}' multiple times. Target '${arg_TARGET}', mode '${CMAKE_BUILD_TYPE}'")
        endif()
        return ()
    endif()
    list(APPEND LOCAL_COPIED_RESOURCES "${COPY_PAIR}")
    set_property(GLOBAL PROPERTY COPIED_RESOURCES ${LOCAL_COPIED_RESOURCES})

    # file-glob for resource_dir with file-extensions
    set(RESOURCE_PATTERNS "")
    list(LENGTH arg_FILE_EXTENSIONS COUNT_FILE_EXTENSIONS)
    if (${COUNT_FILE_EXTENSIONS} EQUAL 0) # no file-extension; take all
        list(APPEND RESOURCE_PATTERNS "${arg_RESOURCE_DIR}/*")
    else ()
        foreach (FILE_EXTENSION ${arg_FILE_EXTENSIONS})
            list(APPEND RESOURCE_PATTERNS "${arg_RESOURCE_DIR}/${FILE_EXTENSION}")
        endforeach ()
    endif ()
    file(GLOB_RECURSE RESOURCE_FILES
        ${RESOURCE_PATTERNS}
    )
    # filter excluded dirs
    list(LENGTH arg_EXCLUDE COUNT_EXCLUDE)
    if (${COUNT_EXCLUDE} GREATER 0)
        set(EXCLUDE_DIRS "")
        foreach (EXCLUDE_DIR ${arg_EXCLUDE})
            escape_regex(${EXCLUDE_DIR} EXCLUDE_DIR_SANITIZED)
            list(APPEND EXCLUDE_DIRS "^${EXCLUDE_DIR_SANITIZED}")
        endforeach ()
        list(JOIN EXCLUDE_DIRS "|" EXCLUDE_PATTERN)
        list(FILTER RESOURCE_FILES EXCLUDE REGEX "${EXCLUDE_PATTERN}")
    endif ()

    # add copy commands for all files
    set(OUTPUT_FILES "")
    foreach (RESOURCE_FILE ${RESOURCE_FILES})
        file(RELATIVE_PATH REL_PATH "${arg_RESOURCE_DIR}" "${RESOURCE_FILE}")
        set(TARGET_FILE "${arg_TARGET_DIR}/${REL_PATH}")

        file(RELATIVE_PATH REL_RESOURCE_FILE_WKS "${WORKSPACE_DIR}" "${RESOURCE_FILE}")
        file(RELATIVE_PATH REL_TARGET_FILE_WKS "${WORKSPACE_DIR}" "${TARGET_FILE}")

        add_custom_command(
            OUTPUT "${TARGET_FILE}"
            COMMAND ${CMAKE_COMMAND} -E copy "${RESOURCE_FILE}" "${TARGET_FILE}"
            DEPENDS "${RESOURCE_FILE}"
            COMMENT "Copy '${REL_RESOURCE_FILE_WKS}' -> '${REL_TARGET_FILE_WKS}'"
            VERBATIM
        )
        message(STATUS "Added copy-command '${REL_RESOURCE_FILE_WKS}' -> '${REL_TARGET_FILE_WKS}'")
        list(APPEND OUTPUT_FILES "${TARGET_FILE}")
    endforeach ()

    file(RELATIVE_PATH REL_RESOURCE_PATH "${WORKSPACE_DIR}" "${arg_RESOURCE_DIR}")
    string(REPLACE "/" "_" REL_RESOURCE_NAME "${REL_RESOURCE_PATH}")

    set(CUSTOM_COPY_TARGET_NAME "copy_resources-${REL_RESOURCE_NAME}-${CMAKE_BUILD_TYPE}")
    add_custom_target("${CUSTOM_COPY_TARGET_NAME}" ALL DEPENDS ${OUTPUT_FILES})
    add_dependencies("${arg_TARGET}" "${CUSTOM_COPY_TARGET_NAME}")
    list(APPEND COPIED_RESOURCES "${arg_RESOURCE_DIR},${arg_TARGET_DIR}")
endfunction()

set(COPIED_SHADERS "")
function(compile_shaders)
    set(options "ONCE")
    set(oneValueArgs TARGET TARGET_DIR SHADER_DIR)
    set(multiValueArgs "")
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    set(COPY_PAIR "${arg_SHADER_DIR},${arg_TARGET_DIR}")
    get_property(LOCAL_COPIED_SHADERS GLOBAL PROPERTY COPIED_SHADERS)
    list(FIND LOCAL_COPIED_SHADERS "${COPY_PAIR}" COPY_PAIR_IDX)
    if (${COPY_PAIR_IDX} GREATER_EQUAL 0)
        if (NOT arg_ONCE)
            message(WARNING "Tried to compile shaders from '${arg_SHADER_DIR}' to '${arg_TARGET_DIR}' multiple times. Target '${arg_TARGET}', mode '${CMAKE_BUILD_TYPE}'")
        endif()
        return ()
    endif()
    list(APPEND LOCAL_COPIED_SHADERS "${COPY_PAIR}")
    set_property(GLOBAL PROPERTY COPIED_SHADERS ${LOCAL_COPIED_SHADERS})

    file(GLOB_RECURSE SHADER_FILES
        "${arg_SHADER_DIR}/*.vert"
        "${arg_SHADER_DIR}/*.frag"
        "${arg_SHADER_DIR}/*.geom"
        "${arg_SHADER_DIR}/*.comp"
        "${arg_SHADER_DIR}/*.tesc"
        "${arg_SHADER_DIR}/*.tese"
    )

    set(OUTPUT_FILES "")
    foreach (GLSL_FILE ${SHADER_FILES})
        file(RELATIVE_PATH REL_PATH "${arg_SHADER_DIR}" "${GLSL_FILE}")
        set(TARGET_SPIRV_FILE "${arg_TARGET_DIR}/${REL_PATH}.spv")

        file(RELATIVE_PATH REL_GLSL_FILE_WKS "${WORKSPACE_DIR}" "${GLSL_FILE}")
        file(RELATIVE_PATH REL_TARGET_SPIRV_FILE_WKS "${WORKSPACE_DIR}" "${TARGET_SPIRV_FILE}")

        add_custom_command(
            OUTPUT "${TARGET_SPIRV_FILE}"
            COMMAND glslc "${GLSL_FILE}" -o "${TARGET_SPIRV_FILE}"
            DEPENDS "${GLSL_FILE}"
            COMMENT "Compile '${REL_GLSL_FILE_WKS}' -> '${REL_TARGET_SPIRV_FILE_WKS}'"
            VERBATIM
        )
        message(STATUS "Added compile-command '${REL_GLSL_FILE_WKS}' -> '${REL_TARGET_SPIRV_FILE_WKS}'")
        list(APPEND OUTPUT_FILES "${TARGET_SPIRV_FILE}")
    endforeach ()

    file(RELATIVE_PATH REL_SHADER_DIR "${WORKSPACE_DIR}" "${arg_SHADER_DIR}")
    string(REPLACE "/" "_" REL_SHADER_NAME "${REL_SHADER_DIR}")

    set(CUSTOM_SHADER_TARGET_NAME "compile_shaders-${REL_SHADER_NAME}-${CMAKE_BUILD_TYPE}")
    add_custom_target("${CUSTOM_SHADER_TARGET_NAME}" ALL DEPENDS ${OUTPUT_FILES})
    add_dependencies("${arg_TARGET}" "${CUSTOM_SHADER_TARGET_NAME}")
endfunction()