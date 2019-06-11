// libstdaudio
// Copyright (c) 2018 - 2019 - Timur Doumler, Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include "__asio_audio_device.h"

_LIBSTDAUDIO_NAMESPACE_BEGIN

optional<audio_device> get_default_audio_input_device() {
  auto& devices = __asio_devices::get();
  return devices.default_input();
}

optional<audio_device> get_default_audio_output_device() {
  auto& devices = __asio_devices::get();
  return devices.default_output();
}

audio_device_list get_audio_input_device_list() {
  auto& devices = __asio_devices::get();
  return devices.inputs();
}

audio_device_list get_audio_output_device_list() {
  auto& devices = __asio_devices::get();
  return devices.outputs();
}

template<typename F, typename /* = enable_if_t<std::is_nothrow_invocable_v<F>> */>
void set_audio_device_list_callback(F&&) {
  // TODO: implement!
}

_LIBSTDAUDIO_NAMESPACE_END
