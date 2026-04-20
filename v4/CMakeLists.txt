cmake_minimum_required(VERSION 3.20)
project(stream-overlay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(stream-overlay WIN32
    src/main.cpp
    src/renderer.cpp
    src/tcp_server.cpp
    src/bottom_popup.cpp
    src/bbcode.cpp
    src/timer.cpp
    src/sound.cpp
)

target_link_libraries(stream-overlay PRIVATE
    d2d1
    d3d11
    dxgi
    dcomp
    dwrite
    ws2_32
    winmm
)

install(TARGETS stream-overlay RUNTIME DESTINATION bin)
