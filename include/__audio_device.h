// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

_LIBSTDAUDIO_NAMESPACE_BEGIN

template <typename _AudioDriverType>
class audio_basic_device;

template <typename _AudioDriverType>
class audio_basic_device_list;

template <typename _AudioDriverType = audio_default_driver_t>
optional<audio_basic_device<_AudioDriverType>> get_default_audio_input_device();

template <typename _AudioDriverType = audio_default_driver_t>
optional<audio_basic_device<_AudioDriverType>> get_default_audio_output_device();

template <typename _AudioDriverType = audio_default_driver_t>
audio_basic_device_list<_AudioDriverType> get_audio_input_device_list();

template <typename _AudioDriverType = audio_default_driver_t>
audio_basic_device_list<_AudioDriverType> get_audio_output_device_list();

using audio_device = audio_basic_device<audio_default_driver_t>;
using audio_device_list = audio_basic_device_list<audio_default_driver_t>;

_LIBSTDAUDIO_NAMESPACE_END
