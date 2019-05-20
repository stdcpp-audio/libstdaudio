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
#include <functional>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atlbase.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

using __asio_common_sample_type = float;

struct audio_device_exception : public runtime_error {
  explicit audio_device_exception(const char* what)
    : runtime_error(what) {
  }
};

// TODO: templatize audio_device as per 6.4 Device Selection API
class audio_device final {
public:
  using device_id_t = int;

  explicit audio_device(device_id_t id, string name, CLSID class_id)
    : _name(name), _id(id)
  {
    initialise_asio(class_id);
  }

  ~audio_device() {
    stop();
    _asio->disposeBuffers();
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
    return is_same_v<_SampleType, __asio_common_sample_type>;
  }

  constexpr bool can_connect() const noexcept {
    return false;
  }

  constexpr bool can_process() const noexcept {
    return false;
  }

  bool start() {
    _instance = this;
    const auto result = _asio->start();
    _running = result == ASE_OK;
    return _running;
  }

  bool stop() {
    const auto result = _asio->stop();
    _running = false;
    _instance = nullptr;
    return result == ASE_OK;
  }

  bool is_running() const noexcept {
    return _running;
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

  template <typename _CallbackType,
            typename = enable_if_t<is_nothrow_invocable_v<_CallbackType, audio_device&, audio_device_io<__asio_common_sample_type >&>> >
  void connect(_CallbackType callback) {
    if (_running) {
      throw audio_device_exception("cannot connect to running audio_device");
    }
    _user_callback = move(callback);

    for (int i = 0; i < _num_inputs; ++i) {
      _asio_buffers.emplace_back(ASIOBufferInfo{true, i});
    }
    for (int i = 0; i < _num_outputs; ++i) {
      _asio_buffers.emplace_back(ASIOBufferInfo{false, i});
    }

    const long num_channels = _num_inputs + _num_outputs;
    const auto result = _asio->createBuffers(_asio_buffers.data(), num_channels, _buffer_size, &_asio_callbacks);

    if (result != ASE_OK) {
      throw audio_device_exception("failed to create ASIO buffers: " + result);
    }
  }

private:
  void initialise_asio(CLSID class_id) {
    const auto result = CoCreateInstance(class_id, nullptr, CLSCTX_INPROC_SERVER, class_id, reinterpret_cast<void**>(&_asio));
    if (result) {
      throw audio_device_exception("Failed to open ASIO driver: 0x" + result);
    }
    if (!_asio->init(nullptr)) {
      return;
    }

    initialise_callbacks();
    initialise_io();
    initialise_sample_rates();
  }

  void initialise_callbacks() {
    _asio_callbacks.bufferSwitch = &audio_device::buffer_switch;
    _asio_callbacks.bufferSwitchTimeInfo = &audio_device::buffer_switch_time_info;
    _asio_callbacks.asioMessage = &audio_device::asio_message;
    _asio_callbacks.sampleRateDidChange = &audio_device::sample_rate_changed;
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

    ASIOChannelInfo info{0, _num_inputs > 0};
    _asio->getChannelInfo(&info);
    _sample_type = info.type;
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

  static void buffer_switch(long /*index*/, ASIOBool /*direct_process*/) {}


  static void sample_rate_changed(ASIOSampleRate /*sample_rate*/) {}

  static long asio_message(long selector, long /*value*/, void* /*message*/, double* /*opt*/) {
    switch (selector)
    {
    case kAsioSupportsTimeInfo: return ASIOTrue;
    case kAsioEngineVersion: return 2;
    case kAsioSelectorSupported: return ASIOFalse;
    default: return false;
    }
  }

  static ASIOTime* buffer_switch_time_info (ASIOTime* params, long index, ASIOBool /*direct_process*/) {
    return _instance->on_buffer_switch(params, index);
  }

  static ASIOTime* on_buffer_switch(ASIOTime* /*params*/, long /*double_buffer_index*/) {
    return nullptr;
  }

  CComPtr<IASIO> _asio;
  const string _name;
  device_id_t _id;
  long _num_inputs = 0;
  long _num_outputs = 0;
  vector<sample_rate_t> _sample_rates;
  vector<buffer_size_t> _buffer_sizes;
  buffer_size_t _buffer_size = 0;
  bool _running = false;

  ASIOSampleType _sample_type;
  using __asio_callback_t = function<void(audio_device&, audio_device_io<__asio_common_sample_type>&)>;
  __asio_callback_t _user_callback;

  vector< ASIOBufferInfo> _asio_buffers;
  static audio_device* _instance;
  ASIOCallbacks _asio_callbacks;
};

audio_device* audio_device::_instance = nullptr;

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
    return *outputs().begin();
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

      if (is_excluded(name)) {
        continue;
      }
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

  bool is_excluded(string_view name)
  {
    if (name == "Realtek ASIO") {
      return true;
    }
    return false;
  }
};

_LIBSTDAUDIO_NAMESPACE_END
