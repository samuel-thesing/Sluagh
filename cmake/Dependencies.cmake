include(FetchContent)

set(FETCHCONTENT_QUIET OFF) # for showing git clone progress and other

# ==============================================================================
# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    SYSTEM
)

FetchContent_MakeAvailable(spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ==============================================================================
# SDL3
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

if (BUILD_SHARED)
    set(SDL_SHARED ON)
    set(SDL_STATIC OFF)
else ()
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
endif ()

FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG release-3.4.4
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    SYSTEM
)
FetchContent_MakeAvailable(SDL3)

