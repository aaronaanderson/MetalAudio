cmake_minimum_required(VERSION 3.21)

project(METALAUDIO)

set (CMAKE_CXX_STANDARD 17)

add_subdirectory(metal-cmake)

set(RTAUDIO_BUILD_STATIC_LIBS ON CACHE BOOL "Rtaudio Shared Lib" FORCE)
set(RTAUDIO_BUILD_TESTING OFF CACHE BOOL "Rtaudio Testing" FORCE)
set(RTAUDIO_TARGETNAME_UNINSTALL
    RTAUDIO_UNINSTALL CACHE STRING "Rtaudio Uninstall Target" FORCE)
add_subdirectory(rtaudio)

add_executable(MetalAudio "Main.cpp")

set_target_properties(MetalAudio PROPERTIES 
    DEBUG_POSTFIX _DEBUG 
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(MetalAudio PRIVATE METAL_CPP rtaudio)