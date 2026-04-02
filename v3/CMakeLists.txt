cmake_minimum_required(VERSION 3.20)
project(stream-overlay VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.9
    GIT_SHALLOW    TRUE
)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG        v2.4.1
    GIT_SHALLOW    TRUE
)

set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-2.22.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(SDL2 box2d SDL2_ttf)

add_executable(stream-overlay
    src/main.cpp
    src/PhysicsMode.cpp
    src/PopUpsMode.cpp
    src/BottomPopupMode.cpp
)

target_link_libraries(stream-overlay PRIVATE SDL2::SDL2 SDL2::SDL2main box2d SDL2_ttf::SDL2_ttf)

# Copy SDL2 DLL next to the executable on Windows
if(WIN32)
    add_custom_command(TARGET stream-overlay POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:stream-overlay>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2_ttf::SDL2_ttf>
            $<TARGET_FILE_DIR:stream-overlay>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:box2d>
            $<TARGET_FILE_DIR:stream-overlay>
    )
endif()
