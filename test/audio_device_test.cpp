// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include <set>
#include <thread>
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE("User cannot instantiate a device")
{
  CHECK_FALSE(std::is_default_constructible_v<audio_device>);
}

TEST_CASE("All devices have non-empty names")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    CHECK_FALSE(device.name().empty());
  }
}

TEST_CASE("All input devices have unique IDs")
{
  auto devices = get_audio_input_device_list();
  std::set<audio_device::device_id_t> device_ids;
  for (auto& device : devices) {
    auto insert_result = device_ids.insert(device.device_id());
    CHECK(insert_result.second);
  }
}

TEST_CASE("All output devices have unique IDs")
{
  auto devices = get_audio_output_device_list();
  std::set<audio_device::device_id_t> device_ids;
  for (auto& device : devices) {
    auto insert_result = device_ids.insert(device.device_id());
    CHECK(insert_result.second);
  }
}

TEST_CASE("If there are any input devices, there must be a default input device")
{
  auto default_device = get_default_audio_input_device();
  auto devices = get_audio_input_device_list();
  if (devices.empty()) {
    CHECK_FALSE(default_device.has_value());
  }
  else {
    CHECK(default_device.has_value());
  }
}

TEST_CASE("If there are any output devices, there must be a default output device")
{
  auto default_device = get_default_audio_output_device();
  auto devices = get_audio_output_device_list();
  if (devices.empty()) {
    CHECK_FALSE(default_device.has_value());
  }
  else {
    CHECK(default_device.has_value());
  }
}

TEST_CASE("The default input device is contained in the input devices")
{
  auto default_device = get_default_audio_input_device();
  auto devices = get_audio_input_device_list();
  if (default_device.has_value()) {
    auto result = std::find_if(devices.begin(), devices.end(), [&default_device](const audio_device &device) {
      return device.device_id() == default_device->device_id();
    });
    CHECK_FALSE(result == devices.end());
  }
}

TEST_CASE("The default output device is contained in the output devices")
{
  auto default_device = get_default_audio_output_device();
  auto devices = get_audio_output_device_list();
  if (default_device.has_value()) {
    auto result = std::find_if(devices.begin(), devices.end(), [&default_device](const audio_device &device) {
      return device.device_id() == default_device->device_id();
    });
    CHECK_FALSE(result == devices.end());
  }
}

TEST_CASE("All input devices support input")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    CHECK(device.is_input());
  }
}

TEST_CASE("All output devices support output")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    CHECK(device.is_output());
  }
}

TEST_CASE("All input devices have at least one input channel")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    CHECK(device.get_num_input_channels() > 0);
  }
}

TEST_CASE("All input devices have zero or more output channels")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    CHECK(device.get_num_output_channels() >= 0);
  }
}

TEST_CASE("All output devices have zero or more input channels")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    CHECK(device.get_num_input_channels() >= 0);
  }
}

TEST_CASE("All output devices have at least one output channel")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    CHECK(device.get_num_output_channels() > 0);
  }
}

TEST_CASE("The sample rate type is arithmetic")
{
  CHECK(std::is_arithmetic_v<audio_device::sample_rate_t >);
}

TEST_CASE("All input devices have a positive sample rate")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    CHECK(device.get_sample_rate() > 0);
  }
}

TEST_CASE("All output devices have a positive sample rate")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    CHECK(device.get_sample_rate() > 0);
  }
}

TEST_CASE("Setting a supported sample rate on input devices")
{
  // TODO: this needs to wait until we have an API to query supported settings
}

TEST_CASE("Setting a supported sample rate on output devices")
{
  // TODO: this needs to wait until we have an API to query supported settings
}

TEST_CASE("The buffer size type is integral")
{
  CHECK(std::is_integral_v<audio_device::buffer_size_t>);
}

TEST_CASE("All input devices have a positive buffer size")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    CHECK(device.get_buffer_size_frames() > 0);
  }
}

TEST_CASE("All output devices have a positive buffer size")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    CHECK(device.get_buffer_size_frames() > 0);
  }
}

TEST_CASE("Setting a supported buffer size on input devices")
{
  // TODO: this needs to wait until we have an API to query supported settings
}

TEST_CASE("Setting a supported buffer size on output devices")
{
  // TODO: this needs to wait until we have an API to query supported settings
}

TEST_CASE("On any platform that supports multithreading, all input devices must support connect")
{
  if (std::thread::hardware_concurrency()) {
    auto devices = get_audio_input_device_list();
    for (auto& device : devices) {
      CHECK(device.can_connect());
    }
  }
}

TEST_CASE("On any platform that supports multithreading, all output devices must support connect")
{
  if (std::thread::hardware_concurrency()) {
    auto devices = get_audio_output_device_list();
    for (auto& device : devices) {
      CHECK(device.can_connect());
    }
  }
}

TEST_CASE("Input devices must not be running initially")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    CHECK_FALSE(device.is_running());
  }
}

TEST_CASE("Output devices must not be running initially")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    CHECK_FALSE(device.is_running());
  }
}

TEST_CASE("Input devices must be running if started successfully")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    if (device.start()) {
      CHECK(device.is_running());
    }
  }
}

TEST_CASE("Output devices must be running if started successfully")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    if (device.start()) {
      CHECK(device.is_running());
    }
  }
}

TEST_CASE("Input devices must not be running after stopped successfully")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    device.start();
    if (device.stop()) {
      CHECK_FALSE(device.is_running());
    }
  }
}

TEST_CASE("Output devices must not be running after stopped successfully")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    device.start();
    if (device.stop()) {
      CHECK_FALSE(device.is_running());
    }
  }
}

TEST_CASE("Repeatedly starting an input device is not an error")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    device.start();
    device.start();
    device.start();
    device.start();
    if (device.start()) {
      CHECK(device.is_running());
    }
  }
}

TEST_CASE("Repeatedly starting an output device is not an error")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    device.start();
    device.start();
    device.start();
    device.start();
    if (device.start()) {
      CHECK(device.is_running());
    }
  }
}

TEST_CASE("Repeatedly stopping an input device is not an error")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    device.start();
    device.stop();
    device.stop();
    device.stop();
    device.stop();
    if (device.stop()) {
      CHECK_FALSE(device.is_running());
    }
  }
}

TEST_CASE("Repeatedly stopping an output device is not an error")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    device.start();
    device.stop();
    device.stop();
    device.stop();
    device.stop();
    if (device.stop()) {
      CHECK_FALSE(device.is_running());
    }
  }
}

#ifndef _WIN32
TEST_CASE("Starting an input device and registering a start callback") {
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    auto start_cb = [](audio_device&){};
    device.start(std::move(start_cb));
  }

  // TODO: start_cb should actually be called (sync or async!)
}

TEST_CASE("Starting an output device and registering a start callback") {
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    auto start_cb = [](audio_device&){};
    device.start(std::move(start_cb));
  }

  // TODO: start_cb should actually be called (sync or async!)
}

TEST_CASE("Starting an input device and registering both a start and a stop callback") {
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    auto start_cb = [](audio_device&){};
    auto stop_cb = [](audio_device&){};
    device.start(std::move(start_cb), std::move(stop_cb));
  }

  // TODO: start_cb and stop_cb should actually be called (sync or async!)
}


TEST_CASE("Starting an output device and registering both a start and a stop callback") {
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    auto start_cb = [](audio_device&){};
    auto stop_cb = [](audio_device&){};
    device.start(std::move(start_cb), std::move(stop_cb));
  }

  // TODO: start_cb and stop_cb should actually be called (sync or async!)
}
#endif

TEST_CASE("Stopping an input device that is not running must succeed")
{
  auto devices = get_audio_input_device_list();
  for (auto& device : devices) {
    CHECK (device.stop());
  }
}

TEST_CASE("Stopping an output device that is not running must succeed")
{
  auto devices = get_audio_output_device_list();
  for (auto& device : devices) {
    CHECK (device.stop());
  }
}

#ifndef _WIN32
TEST_CASE("Register device list change callback")
{
  auto cb = []{};
  set_audio_device_list_callback(audio_device_list_event::device_list_changed, cb);
}

TEST_CASE("Register default input device change callback")
{
  auto cb = []{};
  set_audio_device_list_callback(audio_device_list_event::default_input_device_changed, cb);
}

TEST_CASE("Register default output device change callback")
{
  auto cb = []{};
  set_audio_device_list_callback(audio_device_list_event::default_output_device_changed, cb);
}
#endif
