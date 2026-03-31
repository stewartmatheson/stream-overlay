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
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG        v2.4.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(box2d)

add_executable(stream-overlay
    src/main.cpp
    src/PhysicsMode.cpp
)

target_link_libraries(stream-overlay PRIVATE SDL2::SDL2 SDL2::SDL2main box2d)

# Copy SDL2 DLL next to the executable on Windows
if(WIN32)
    add_custom_command(TARGET stream-overlay POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:stream-overlay>
    )
endif()
