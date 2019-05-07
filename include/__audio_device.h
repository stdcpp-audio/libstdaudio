// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

_LIBSTDAUDIO_NAMESPACE_BEGIN

class audio_device;
class audio_device_list;

optional<audio_device> get_default_audio_input_device();
optional<audio_device> get_default_audio_output_device();

audio_device_list get_audio_input_device_list();
audio_device_list get_audio_output_device_list();

_LIBSTDAUDIO_NAMESPACE_END
