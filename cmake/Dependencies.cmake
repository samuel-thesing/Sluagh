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

# ==============================================================================
# SDL3_image

set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_GIF OFF)
set(SDLIMAGE_JPG OFF)
set(SDLIMAGE_JXL OFF)
set(SDLIMAGE_LBM OFF)
set(SDLIMAGE_PCX OFF)
set(SDLIMAGE_PNM OFF)
set(SDLIMAGE_QOI OFF)
set(SDLIMAGE_SVG OFF)
set(SDLIMAGE_TGA OFF)
set(SDLIMAGE_TIF OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_XPM OFF)
set(SDLIMAGE_XCF OFF)
set(SDLIMAGE_XV OFF)


FetchContent_Declare(SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
    GIT_TAG release-3.4.4
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    SYSTEM
)
FetchContent_MakeAvailable(SDL3_image)