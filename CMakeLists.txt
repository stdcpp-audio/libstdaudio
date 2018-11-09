# libstdaudio
# Copyright (c) 2018 - Timur Doumler

cmake_minimum_required(VERSION 3.12)
project(libstdaudio)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(libstdaudio include/audio.h include/device.h include/config.h)

add_executable(white_noise examples/white_noise.cpp)