// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE("User cannot instantiate a null device")
{
  CHECK_FALSE(std::is_default_constructible_v<audio_basic_device<audio_null_driver_t>>);
}

TEST_CASE("No default input device")
{
  auto device = get_default_audio_input_device<audio_null_driver_t>();
  CHECK_FALSE(device);
}

TEST_CASE("No default output device")
{
  auto device = get_default_audio_output_device<audio_null_driver_t>();
  CHECK_FALSE(device);
}

TEST_CASE("No input devices")
{
  auto devices = get_audio_input_device_list<audio_null_driver_t>();
  CHECK(devices.begin() == devices.end());
  CHECK_FALSE(devices.begin() != devices.end());
  CHECK(devices.empty());

  int count = 0;
  for (auto& device : devices)
    count++;

  CHECK(count == 0);
}

TEST_CASE("No output devices")
{
  auto devices = get_audio_input_device_list<audio_null_driver_t>();
  CHECK(devices.begin() == devices.end());
  CHECK_FALSE(devices.begin() != devices.end());
  CHECK(devices.empty());

  int count = 0;
  for (auto& device : devices)
    count++;

  CHECK(count == 0);
}
