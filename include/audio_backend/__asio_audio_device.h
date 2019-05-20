// libstdaudio
// Copyright (c) 2019 Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>
#include <chrono>
#include <cassert>
#include <forward_list>

_LIBSTDAUDIO_NAMESPACE_BEGIN

class audio_device {
public:
  audio_device() = delete;

  string_view name() const noexcept {
    return {};
  }

  using device_id_t = unsigned;

  device_id_t device_id() const noexcept {
    return {};
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

  using sample_rate_t = int;

  sample_rate_t get_sample_rate() const noexcept {
    return {};
  }

  span<const sample_rate_t> get_supported_sample_rates() const noexcept {
    return {};
  }

  bool set_sample_rate(sample_rate_t) {
    return false;
  }

  using buffer_size_t = int;

  buffer_size_t get_buffer_size_frames() const noexcept {
    return {};
  }

  span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept {
    return {};
  }

  bool set_buffer_size_frames(buffer_size_t) {
    return false;
  }

  template <typename _SampleType>
  constexpr bool supports_sample_type() const noexcept {
    return false;
  }

  constexpr bool can_connect() const noexcept {
    return false;
  }

  constexpr bool can_process() const noexcept {
    return false;
  }

  bool start() {
    return false;
  }

  bool stop() {
    return false;
  }

  bool is_running() const noexcept {
    return false;
  }

  void wait() const {
    assert(false);
  }

  template<class _Rep, class _Period>
  void wait_for(chrono::duration<_Rep, _Period> rel_time) const {
    assert(false);
  }

  template<class _Clock, class _Duration>
  void wait_until(chrono::time_point<_Clock, _Duration> abs_time) const {
    assert(false);
  }

  template <typename _CallbackType>
  void process(_CallbackType&) {
    assert(false);
  }

  template <typename _CallbackType>
  void connect(_CallbackType) {
    assert(false);
  }
};

class audio_device_list : public forward_list<audio_device> {
};

_LIBSTDAUDIO_NAMESPACE_END
