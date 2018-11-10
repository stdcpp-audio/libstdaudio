# libstdaudio
# Copyright (c) 2018 - Timur Doumler

cmake_minimum_required(VERSION 3.12)
project(libstdaudio)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(libstdaudio include/audio.h include/device.h include/config.h test/device_test.cpp test/test_main.cpp src/device.cpp include/buffer_list.h src/buffer_list.cpp test/buffer_list_test.cpp)

add_executable(white_noise examples/white_noise.cpp)
target_link_libraries(white_noise libstdaudio)

add_executable(test test/test_main.cpp test/device_test.cpp test/buffer_list_test.cpp)
target_link_libraries(test libstdaudio)