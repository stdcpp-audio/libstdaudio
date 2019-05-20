// libstdaudio
// Copyright (c) 2018 - 2019 - Timur Doumler, Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include "__asio_audio_device.h"

_LIBSTDAUDIO_NAMESPACE_BEGIN

// TODO: templatize audio_device as per 6.4 Device Selection API
// This will allow us to select between ASIO & WASAPI
optional<audio_device> get_default_audio_input_device() {
  return {};
}

optional<audio_device> get_default_audio_output_device() {
  return {};
}

audio_device_list get_audio_input_device_list() {
  return {};
}

audio_device_list get_audio_output_device_list() {
  return {};
}

_LIBSTDAUDIO_NAMESPACE_END
