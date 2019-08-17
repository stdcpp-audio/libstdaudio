// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>
#include <chrono>
#include <cassert>

_LIBSTDAUDIO_NAMESPACE_BEGIN

struct audio_device_exception : public runtime_error {
  explicit audio_device_exception(const char* what)
    : runtime_error(what) {
  }
};

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

  using sample_rate_t = unsigned;

  sample_rate_t get_sample_rate() const noexcept {
    return {};
  }

  bool set_sample_rate(sample_rate_t) {
    return false;
  }

  using buffer_size_t  = unsigned;

  buffer_size_t get_buffer_size_frames() const noexcept {
    return {};
  }

  bool set_buffer_size_frames([[maybe_unused]] buffer_size_t new_buffer_size) {
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

  template <typename _CallbackType,
           enable_if_t<is_nothrow_invocable_v<_CallbackType, audio_device&, audio_device_io<float>&>, int> = 0>
  void connect(_CallbackType callback)
  {
    throw audio_device_exception("cannot connect to a null-backend audio_device");
  }

  // TODO: remove std::function as soon as C++20 default-ctable lambda and lambda in unevaluated contexts become available
  using no_op_t = std::function<void(audio_device &)>;

  template <typename _StartCallbackType = no_op_t,
            typename _StopCallbackType = no_op_t,
            // TODO: is_nothrow_invocable_t does not compile, temporarily replaced with is_invocable_t
            typename = enable_if_t<is_invocable_v<_StartCallbackType, audio_device&> && is_invocable_v<_StopCallbackType, audio_device&>>>
  bool start(_StartCallbackType&& start_callback = [](audio_device&) noexcept {},
             _StopCallbackType&& stop_callback = [](audio_device&) noexcept {}) {
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

  template <typename _CallbackType>
  void process(_CallbackType&) {
    assert(false);
  }

  constexpr bool has_unprocessed_io() const noexcept {
    return false;
  }
};

class audio_device_list
{
private:
  class iterator {
  public:
    auto operator==(const iterator&) const noexcept { return true; }
    auto operator!=(const iterator&) const noexcept { return false; }
    auto operator++() -> const iterator& { assert(false); return *this; }
    auto operator*() -> audio_device& {
      assert(false); static audio_device device{};
      return device;
    }
  };

public:
  auto begin() -> iterator { return {}; }
  auto end() -> iterator { return {}; }
  auto begin() const -> iterator { return {}; }
  auto end() const -> iterator { return {}; }
  auto empty() const -> bool { return true; }
};

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

template <typename F, typename /* = enable_if_t<is_invocable_v<F>> */>
void set_audio_device_list_callback(audio_device_list_event event, F &&callback) {
}

_LIBSTDAUDIO_NAMESPACE_END