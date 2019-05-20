// libstdaudio
// Copyright (c) 2019 Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include "iasiodrv.h"

#include <string_view>
#include <chrono>
#include <cassert>
#include <forward_list>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atlbase.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

// TODO: templatize audio_device as per 6.4 Device Selection API
class audio_device final {
public:
  using device_id_t = int;

  explicit audio_device(device_id_t id, string name, CLSID class_id)
    : _name(name), _id(id)
  {
    initialise_asio(class_id);
  }

  string_view name() const noexcept {
    return _name;
  }

  device_id_t device_id() const noexcept {
    return _id;
  }

  bool is_input() const noexcept {
    return _num_inputs > 0;
  }

  bool is_output() const noexcept {
    return _num_outputs > 0;
  }

  int get_num_input_channels() const noexcept {
    return _num_inputs;
  }

  int get_num_output_channels() const noexcept {
    return _num_outputs;
  }

  using sample_rate_t = ASIOSampleRate;

  sample_rate_t get_sample_rate() const noexcept {
    sample_rate_t sample_rate = 0;
    _asio->getSampleRate(&sample_rate);
    return sample_rate;
  }

  span<const sample_rate_t> get_supported_sample_rates() const noexcept {
    return _sample_rates;
  }

  bool set_sample_rate(sample_rate_t sample_rate) {
    const auto result = _asio->setSampleRate(sample_rate);
    return result == ASE_OK;
  }

  using buffer_size_t = long;

  buffer_size_t get_buffer_size_frames() const noexcept {
    return _buffer_size;
  }

  span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept {
    return _buffer_sizes;
  }

  bool set_buffer_size_frames(const buffer_size_t buffer_size) {
    if (find(begin(_buffer_sizes), end(_buffer_sizes), buffer_size) == end(_buffer_sizes)) {
      return false;
    }
    _buffer_size = buffer_size;
    return true;
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

private:
  void initialise_asio(CLSID class_id) {
    const auto result = CoCreateInstance(class_id, nullptr, CLSCTX_INPROC_SERVER, class_id, reinterpret_cast<void**>(&_asio));
    if (result) {
      throw runtime_error("Failed to open ASIO driver: 0x" + result);
    }
    if (!_asio->init(nullptr)) {
      return;
    }

    initialise_io();
    initialise_sample_rates();
  }

  void initialise_io() {
    const auto result = _asio->getChannels(&_num_inputs, &_num_outputs);

    if (result != ASE_OK) {
      return;
    }

    long min;
    long max;
    long granularity;
    _asio->getBufferSize(&min, &max, &_buffer_size, &granularity);

    if (granularity == -1) {
      // Buffers are powers of two
      for (long size = min; size < max; size *= 2) {
        _buffer_sizes.push_back(size);
      }
    }
    else if (granularity == 0) {
      _buffer_sizes.push_back(_buffer_size);
    }
    else if (granularity > 0) {
      for (long size = min; size <= max; size += granularity) {
        _buffer_sizes.push_back(size);
      }
    }
  }

  void initialise_sample_rates()
  {
    constexpr array<sample_rate_t, 6> common_sample_rates = { 44'100, 48'000, 88'200, 96'000, 176'400, 192'000 };

    for (const auto sample_rate : common_sample_rates) {
      if (ASE_OK == _asio->canSampleRate(sample_rate)) {
        _sample_rates.push_back(sample_rate);
      }
    }
    if (0 == get_sample_rate()) {
      set_sample_rate(_sample_rates[0]);
    }
  }

  CComPtr<IASIO> _asio;
  const string _name;
  device_id_t _id;
  long _num_inputs = 0;
  long _num_outputs = 0;
  vector<sample_rate_t> _sample_rates;
  vector<buffer_size_t> _buffer_sizes;
  buffer_size_t _buffer_size = 0;
};

class audio_device_list : public forward_list<audio_device> {
};

class __reg_key_reader final
{
public:
  __reg_key_reader(HKEY key, const string& subkey) {
    const auto result = RegOpenKeyExA(key, subkey.c_str(), 0, KEY_READ, &_key);

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

    for (DWORD index = 0; ; ++index) {
      const auto result = RegEnumKeyEx(_key, index, name, &size, nullptr, nullptr, nullptr, nullptr);

      if (result != ERROR_SUCCESS) {
        break;
      }

      keys.emplace_back(name);
    }

    return keys;
  }

  __reg_key_reader subkey(const string& name)
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

  ~__asio_devices() {
    CoUninitialize();
  }

  optional<audio_device> default_input() {
    return {};
  }

  optional<audio_device> default_output() {
    return {};
  }

  audio_device_list inputs() {
    return enumerate([](const audio_device& device) {
      return device.is_input();
    });
  }

  audio_device_list outputs() {
    return enumerate([](const audio_device& device) {
      return device.is_output();
    });
  }

private:
  __asio_devices() {
    CoInitialize(nullptr);
  }

  template <typename Condition>
  audio_device_list enumerate(Condition condition) {
    audio_device_list devices;

    __reg_key_reader asio_reg(HKEY_LOCAL_MACHINE, "software\\asio");

    int index = 0;
    for (const auto& name : asio_reg.subkeys()) {
      const auto key = asio_reg.subkey(name);
      const auto value = key.value("CLSID");

      CLSID class_id;
      CLSIDFromString(CComBSTR(value.c_str()), &class_id);

      auto device = audio_device{index++, name, class_id};
      if (condition(device)) {
        devices.emplace_front(move(device));
      }
    }
    return devices;
  }
};

_LIBSTDAUDIO_NAMESPACE_END
