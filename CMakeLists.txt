# libstdaudio
# Copyright (c) 2018 - Timur Doumler
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.12)
project(libstdaudio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -O0 -fsanitize=undefined")
if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "-framework CoreAudio")
endif ()

# add the headers to at least one target so that they get
# parsed as part of the project in some IDEs
set(headers
    include/__audio_base.h
    include/__audio_buffer.h
    include/__audio_device.h
    include/__audio_exception.h
    include/__dynamic_library.h
    include/audio
    include/cpp/span.hpp
    include/audio_backend/__coreaudio_backend.h
    include/audio_backend/__null_backend.h
    include/audio_backend/__portaudio_backend.h
    include/audio_backend/__pulseaudio_backend.h
    )
include_directories(include)

add_executable(white_noise examples/white_noise.cpp ${headers})
target_link_libraries(white_noise PRIVATE ${CMAKE_DL_LIBS})
add_executable(print_devices examples/print_devices.cpp)
target_link_libraries(print_devices PRIVATE ${CMAKE_DL_LIBS})
add_executable(sine_wave examples/sine_wave.cpp)
target_link_libraries(sine_wave PRIVATE ${CMAKE_DL_LIBS})
add_executable(melody examples/melody.cpp)
target_link_libraries(melody PRIVATE ${CMAKE_DL_LIBS})
add_executable(level_meter examples/level_meter.cpp)
target_link_libraries(level_meter PRIVATE ${CMAKE_DL_LIBS})

add_executable(test
        test/test_main.cpp
      #  test/null_backend_test.cpp // TODO: fix linker error when uncommenting this one!
        test/default_backend_test.cpp)

target_link_libraries(test PRIVATE ${CMAKE_DL_LIBS})
