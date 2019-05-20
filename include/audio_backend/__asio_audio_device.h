// libstdaudio
// Copyright (c) 2019 Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>
#include <chrono>
#include <cassert>
#include <forward_list>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

// TODO: templatize audio_device as per 6.4 Device Selection API
class audio_device final {
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

class __reg_key_reader final
{
public:
  __reg_key_reader(HKEY key, const char* subkey) {
    const auto result = RegOpenKeyExA(key, subkey, 0, KEY_READ, &_key);

    if (ERROR_SUCCESS != result) {
      throw runtime_error("Failed to read info from registry: 0x" + result);
    }
  }

  ~__reg_key_reader() {
    if (_key) {
      RegCloseKey(_key);
    }
  }

  vector<string> subkeys() const {
    constexpr int max_asio_name_length{32};
    char name[max_asio_name_length];
    DWORD size{max_asio_name_length};
    vector<string> keys;

    for (DWORD index{0}; ; ++index) {
      const auto result = RegEnumKeyEx(_key, index, name, &size, nullptr, nullptr, nullptr, nullptr);

      if (result != ERROR_SUCCESS) {
        break;
      }

      keys.emplace_back(name);
    }

    return keys;
  }

  __reg_key_reader subkey(const char* name)
  {
    return {_key, name};
  }

  string value(const char* name) const {
    DWORD size{0};
    RegQueryValueExA(_key, name, nullptr, nullptr, nullptr, &size);

    string value(size + 1, 0);
    RegQueryValueExA(_key, name, nullptr, nullptr, LPBYTE(value.data()), &size);

    return value;
  }

private:
  HKEY _key{0};
};

class __asio_devices {
public:
  static __asio_devices& get() {
    static __asio_devices instance;
    return instance;
  }

  optional<audio_device> default_input() {
    return {};
  }

  optional<audio_device> default_output() {
    return {};
  }

  audio_device_list inputs() {
    return {};
  }

  audio_device_list outputs() {
    return {};
  }

private:
  __asio_devices()
  {
    enumerate();
  }

  void enumerate()
  {
    __reg_key_reader asio_reg(HKEY_LOCAL_MACHINE, "software\\asio");

    for (const auto& name : asio_reg.subkeys()) {
      auto key = asio_reg.subkey(name.c_str());
      auto class_id = key.value("CLSID");
      class_id = class_id;
    }
  }

};

_LIBSTDAUDIO_NAMESPACE_END
