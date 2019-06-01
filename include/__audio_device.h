// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

_LIBSTDAUDIO_NAMESPACE_BEGIN

class audio_device;
class audio_device_list;

inline optional<audio_device> get_default_audio_input_device();
inline optional<audio_device> get_default_audio_output_device();

inline audio_device_list get_audio_input_device_list();
inline audio_device_list get_audio_output_device_list();

template <typename F, typename = enable_if_t<std::is_invocable_v<F>>>
void set_audio_device_list_callback(F&& cb);

_LIBSTDAUDIO_NAMESPACE_END
