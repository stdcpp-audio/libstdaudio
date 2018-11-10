# libstdaudio
# Copyright (c) 2018 - Timur Doumler

cmake_minimum_required(VERSION 3.12)
project(libstdaudio)

set(CMAKE_CXX_STANDARD 17)

if (APPLE)
  add_definitions(-DLIBSTDAUDIO_HAS_BACKEND -DLIBSTDAUDIO_BACKEND_COREAUDIO)
endif (APPLE)

include_directories(include)

add_library(libstdaudio
  include/audio.h include/config.h
  include/device.h include/buffer_list.h include/device_list.h
  src/device.cpp src/buffer_list.cpp
  src/null_backend/device_list.cpp
  src/coreaudio_backend/device_list.cpp src/_device_impl.h)

add_executable(white_noise examples/white_noise.cpp)
target_link_libraries(white_noise libstdaudio)

add_executable(level_meter examples/level_meter.cpp)
target_link_libraries(level_meter libstdaudio)

add_executable(print_devices examples/print_devices.cpp)
target_link_libraries(print_devices libstdaudio)

add_executable(test test/test_main.cpp test/device_test.cpp test/buffer_list_test.cpp)
target_link_libraries(test libstdaudio)