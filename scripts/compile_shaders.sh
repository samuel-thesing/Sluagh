#!/bin/bash

compile_shaders() {
    local source_dir="$1"
    local output_dir="$source_dir/compiled"

    local success=0

    shopt -s globstar nullglob
    find "$source_dir" -type f | while read -r file; do
        local rel="${file#"$source_dir"/}"
        [[ "$rel" == "compiled"/* ]] && continue;

        local compiled_name="${file/#$source_dir/$output_dir}.spv"
        local compiled_dir
        compiled_dir=$(dirname "$compiled_name")
        if ! mkdir -p "$compiled_dir"; then
            success=1
            continue
        fi

        if ! glslc "$file" -o "$compiled_name"; then
            success=1
        fi
    done

    return $success
}

if ! command -v glslc &>/dev/null; then
    echo "Error: glslc not found" >&2
    exit 1
fi

if ! compile_shaders "./rsc/shader" || ! compile_shaders "./app/rsc/shader"; then
    exit 1
else
    exit 0
fi