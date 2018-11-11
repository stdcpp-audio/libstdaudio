# libstdaudio
# Copyright (c) 2018 - Timur Doumler

cmake_minimum_required(VERSION 3.12)
project(libstdaudio)

set(CMAKE_CXX_STANDARD 17)

if(APPLE)
  add_definitions(-DLIBSTDAUDIO_BACKEND_COREAUDIO)
  set(CMAKE_EXE_LINKER_FLAGS "-framework CoreAudio")
else()
  add_definitions(-DLIBSTDAUDIO_BACKEND_NONE)
endif()

include_directories(include)

add_library(libstdaudio
  include/audio.h include/config.h
  include/device.h include/buffer_list.h include/_backend_null.h
  src/device.cpp src/buffer_list.cpp
  src/_backend_null.cpp src/_backend_coreaudio.cpp src/_device_impl.h)

add_executable(white_noise examples/white_noise.cpp)
target_link_libraries(white_noise libstdaudio)

add_executable(level_meter examples/level_meter.cpp)
target_link_libraries(level_meter libstdaudio)

add_executable(print_devices examples/print_devices.cpp)
target_link_libraries(print_devices libstdaudio)

add_executable(test test/test_main.cpp test/device_test.cpp test/buffer_list_test.cpp)
target_link_libraries(test libstdaudio)