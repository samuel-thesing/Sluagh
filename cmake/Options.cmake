# adds error on warning and all warnings
add_library(project_warnings INTERFACE)

target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
    /W4 /WX /permissive-
    /wd4251
    >
    $<$<CXX_COMPILER_ID:Clang>:
    -Wall -Wextra -Wpedantic
    -Wshadow -Wnon-virtual-dtor
    -Wold-style-cast -Wcast-align
    -Wunused -Woverloaded-virtual
    -Wconversion -Wsign-conversion
    -Werror
    >
)

# adds optimizations on release
add_library(project_optimizations INTERFACE)
target_compile_options(project_optimizations INTERFACE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:
    /O2
    /GL
    >
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Release>>:
    -O3
    -march=native
    -ffunction-sections
    -fdata-sections
    >
)

target_link_options(project_optimizations INTERFACE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:
    /LTCG
    /OPT:REF
    /OPT:ICF
    >
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Release>>:
    -Wl,--gc-sections
    -flto
    -fuse-ld=lld
    -s
    >
)


