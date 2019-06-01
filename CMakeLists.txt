# libstdaudio
# Copyright (c) 2018 - Timur Doumler
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.12)
project(libstdaudio)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif ()

if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "-framework CoreAudio")
endif ()

include_directories(include)

add_executable(white_noise examples/white_noise.cpp)
add_executable(print_devices examples/print_devices.cpp)
add_executable(sine_wave examples/sine_wave.cpp)
add_executable(melody examples/melody.cpp)
add_executable(level_meter examples/level_meter.cpp)

add_executable(test
        test/test_main.cpp
        test/audio_buffer_test.cpp
        test/audio_device_test.cpp)