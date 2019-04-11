// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once
#include <forward_list>
_LIBSTDAUDIO_NAMESPACE_BEGIN

template<>
class audio_basic_device<audio_null_driver_t>
{
public:
  audio_basic_device() = delete;

  using sample_rate_t = double;
  sample_rate_t get_sample_rate() const noexcept { return 0; }
  bool set_sample_rate(sample_rate_t new_sample_rate) {
    return false;
  }

  bool start() { return false; }
  bool stop() { return false; }

  using buffer_size_t = uint32_t;

  buffer_size_t get_buffer_size_frames() const noexcept
  {
      return 0;
  }

  string_view name() const noexcept {
      return {};
  }

  using device_id_t = int;
  device_id_t device_id() const noexcept {
    return -1;
  }

  bool is_input() const noexcept {
    return false;
  }

  bool is_output() const noexcept {
      return false;
  }
  int get_num_input_channels() const noexcept {
    return 0;
  }

  int get_num_output_channels() const noexcept {
    return 0;
  }

  template<typename T>
  void connect(T) {

  }

  constexpr bool can_connect() const noexcept {
    return false;
  }

  constexpr bool can_process() const noexcept {
    return false;
  }

  constexpr bool is_running() const noexcept {
    return false;
  }

  span<const sample_rate_t> get_supported_sample_rates() const noexcept {
    return { };
  }

  span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept {
      return { };
  }

  bool set_buffer_size_frames(buffer_size_t new_buffer_size) {
    return false;
  }

};

using __null_device = audio_basic_device<audio_null_driver_t>;
template<>
class audio_basic_device_list<audio_null_driver_t> : public forward_list<__null_device>
{
};

template<>
auto get_default_audio_input_device<audio_null_driver_t>()
  -> optional<audio_basic_device<audio_null_driver_t>>
{
  return {};
}

template<>
auto get_default_audio_output_device<audio_null_driver_t>()
  -> optional<audio_basic_device<audio_null_driver_t>>
{
  return {};
}

template<>
auto get_audio_input_device_list<audio_null_driver_t>()
  -> audio_basic_device_list<audio_null_driver_t>
{
  return {};
}

template<>
auto get_audio_output_device_list<audio_null_driver_t>()
  -> audio_basic_device_list<audio_null_driver_t>
{
  return {};
}

_LIBSTDAUDIO_NAMESPACE_END
