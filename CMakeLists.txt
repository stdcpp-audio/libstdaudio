# libstdaudio
# Copyright (c) 2018 - Timur Doumler

cmake_minimum_required(VERSION 3.12)
project(libstdaudio)

set(CMAKE_CXX_STANDARD 17)

if (APPLE)
    add_definitions(-DLIBSTDAUDIO_BACKEND_COREAUDIO)
    set(CMAKE_EXE_LINKER_FLAGS "-framework CoreAudio")
else ()
    add_definitions(-DLIBSTDAUDIO_BACKEND_NONE)
endif ()

include_directories(include)

add_library(libstdaudio
        src/device.cpp
        src/device_impl.h
        src/buffer.cpp
        src/buffer_list.cpp
        src/backend/coreaudio_backend.cpp
        src/backend/null_backend.cpp)

add_executable(white_noise examples/white_noise.cpp)
target_link_libraries(white_noise libstdaudio)

add_executable(sine_wave examples/sine_wave.cpp)
target_link_libraries(sine_wave libstdaudio)

add_executable(level_meter examples/level_meter.cpp)
target_link_libraries(level_meter libstdaudio)

add_executable(print_devices examples/print_devices.cpp)
target_link_libraries(print_devices libstdaudio)

add_executable(test
        test/test_main.cpp
        test/buffer_test.cpp
        test/buffer_list_test.cpp
        test/buffer_view_test.cpp
        test/device_test.cpp
        test/strided_span_test.cpp)

target_link_libraries(test libstdaudio)