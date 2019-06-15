// libstdaudio
// Copyright (c) 2019 Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include "__asio_sample.h"

#include "iasiodrv.h"

#include <cassert>
#include <chrono>
#include <forward_list>
#include <functional>
#include <string_view>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atlbase.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

using __asio_common_sample_t = float;

struct audio_device_exception : public runtime_error {
  explicit audio_device_exception(const char* what) : runtime_error(what) {}
};

enum class audio_direction { in, out, full_duplex };

class audio_device final {
public:
  using device_id_t = int;

  explicit audio_device(device_id_t id, string name, IASIO* asio, audio_direction direction)
    : _name(std::move(name)), _id(id), _asio(asio) {
    prepare_asio(direction);
  }

  ~audio_device() {
    stop();
    if (!_asio_buffers.empty()) {
      _asio->disposeBuffers();
    }
    instance(nullptr);
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

  template<class _SampleType>
  constexpr bool supports_sample_type() const noexcept {
    return is_same_v<_SampleType, __asio_common_sample_t>;
  }

  constexpr bool can_connect() const noexcept {
    return true;
  }

  constexpr bool can_process() const noexcept {
    return false;
  }

  // TODO: remove std::function as soon as C++20 default-ctable lambda and lambda in unevaluated contexts become available
  using no_op_t = std::function<void(audio_device&)>;

  template<class _StartCallbackType = no_op_t,
           class _StopCallbackType = no_op_t,
           // TODO: is_nothrow_invocable_t does not compile, temporarily replaced with is_invocable_t
           class = enable_if_t<is_invocable_v<_StartCallbackType, audio_device&> && is_invocable_v<_StopCallbackType, audio_device&>>>
  bool start(_StartCallbackType&& start_callback = [](audio_device&) noexcept {},
             _StopCallbackType&& stop_callback = [](audio_device&) noexcept {}) {

    // TODO: Implement as per P1386R2 (when published)
    const auto success = start();
    if (success) {
      start_callback(*this);
    } else {
      stop_callback(*this);
    }
    return success;
  }

  bool start() {
    if (_asio_buffers.empty()) {
      return false;
    }
    instance(this);
    _sample_position = 0;
    const auto result = _asio->start();
    _running = result == ASE_OK;
    return _running;
  }

  bool stop() {
    const auto result = _asio->stop();
    _running = false;
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

  template<class _CallbackType>
  void process(_CallbackType&) {
    assert(false);
  }

  template<class _CallbackType, class = enable_if_t<is_nothrow_invocable_v<_CallbackType, audio_device&, audio_device_io<float>&>>>
  void connect(_CallbackType callback) {
    if (_running) {
      throw audio_device_exception("cannot connect to running audio_device");
    }
    _user_callback = move(callback);

    allocate_buffers();
  }

private:
  void prepare_asio(const audio_direction direction) {

    set_callbacks();
    query_io(direction);
    query_sample_rates();
  }

  void set_callbacks() {
    _asio_callbacks.bufferSwitch = &audio_device::buffer_switch;
    _asio_callbacks.bufferSwitchTimeInfo = &audio_device::buffer_switch_time_info;
    _asio_callbacks.asioMessage = &audio_device::asio_message;
    _asio_callbacks.sampleRateDidChange = &audio_device::sample_rate_changed;
  }

  void query_io(const audio_direction direction) {
    auto result = _asio->getChannels(&_num_inputs, &_num_outputs);

    if (result != ASE_OK) {
      return;
    }

    switch (direction) {
      case audio_direction::in: _num_outputs = 0; break;
      case audio_direction::out: _num_inputs = 0; break;
      default: break;
    }

    long min;
    long max;
    long granularity;
    result = _asio->getBufferSize(&min, &max, &_buffer_size, &granularity);

    if (result != ASE_OK) {
      return;
    }

    switch (granularity) {
      default: set_buffers(min, max, granularity); break;
      case -1: set_buffers_as_powers_of_two(min, max); break;
      case 0: set_buffers_as_default_size_only(); break;
    }

    set_buffer_size_frames(_buffer_size);

    ASIOChannelInfo info{0, is_input()};
    _asio->getChannelInfo(&info);
    _sample_type = info.type;
  }

  void set_buffers(long min, long max, long granularity) {
    for (long size = min; size <= max; size += granularity) {
      _buffer_sizes.push_back(size);
    }
  }

  void set_buffers_as_powers_of_two(long min, long max) {
    for (long size = min; size <= max; size *= 2) {
      _buffer_sizes.push_back(size);
    }
  }

  void set_buffers_as_default_size_only() {
    _buffer_sizes.push_back(_buffer_size);
  }

  void query_sample_rates() {
    constexpr array<sample_rate_t, 6> common_sample_rates = {44'100, 48'000, 88'200, 96'000, 176'400, 192'000};

    for (const auto sample_rate : common_sample_rates) {
      if (ASE_OK == _asio->canSampleRate(sample_rate)) {
        _sample_rates.push_back(sample_rate);
      }
    }
    if (0 == get_sample_rate()) {
      set_sample_rate(_sample_rates[0]);
    }
  }

  void allocate_buffers() {

    for (int i = 0; i < _num_inputs; ++i) {
      _asio_buffers.emplace_back(ASIOBufferInfo{true, i});
    }
    for (int i = 0; i < _num_outputs; ++i) {
      _asio_buffers.emplace_back(ASIOBufferInfo{false, i});
    }

    const long num_channels = _num_inputs + _num_outputs;
    const auto result = _asio->createBuffers(_asio_buffers.data(), num_channels, _buffer_size, &_asio_callbacks);

    if (result != ASE_OK) {
      throw audio_device_exception("failed to create ASIO buffers");
    }

    _input_samples.resize(_num_inputs * _buffer_size);
    _output_samples.resize(_num_outputs * _buffer_size);
    _io.input_buffer = {_input_samples.data(), static_cast<size_t>(_buffer_size), static_cast<size_t>(_num_inputs), contiguous_interleaved};
    _io.output_buffer = {_output_samples.data(), static_cast<size_t>(_buffer_size), static_cast<size_t>(_num_outputs), contiguous_interleaved};

    set_input_reader();
    set_output_writer();
  }

  void set_input_reader() {

    if (!is_input()) {
      _read = [](long) {};
      return;
    }

    switch (_sample_type) {
      case ASIOSTInt32LSB: _read = [this](long index) { read<int32_t>(index); }; break;
      case ASIOSTInt24LSB: _read = [this](long index) { read<packed24_t>(index); }; break;
      case ASIOSTInt16LSB: _read = [this](long index) { read<int16_t>(index); }; break;
      case ASIOSTFloat32LSB: _read = [this](long index) { read<float>(index); }; break;
      case ASIOSTFloat64LSB: _read = [this](long index) { read<double>(index); }; break;

      default: throw audio_device_exception("ASIO native sample type not supported");
    }
  }

  template<class _SampleType>
  void read(long index) {

    auto& in = *_io.input_buffer;
    for (int channel = 0; channel < _num_inputs; ++channel) {
      const auto buffer = static_cast<__asio_sample<_SampleType>*>(_asio_buffers[channel].buffers[index]);
      for (int frame = 0; frame < in.size_frames(); ++frame) {
        in(frame, channel) = buffer[frame].float_value();
      }
    }
  }

  void set_output_writer() {

    if (!is_output()) {
      _write = [](long) {};
      return;
    }

    switch (_sample_type) {
      case ASIOSTInt32LSB: _write = [this](long index) { write<int32_t>(index); }; break;
      case ASIOSTInt24LSB: _write = [this](long index) { write<packed24_t>(index); }; break;
      case ASIOSTInt16LSB: _write = [this](long index) { write<int16_t>(index); }; break;
      case ASIOSTFloat32LSB: _write = [this](long index) { write<float>(index); }; break;
      case ASIOSTFloat64LSB: _write = [this](long index) { write<double>(index); }; break;

      default: throw audio_device_exception("ASIO native sample type not supported");
    }
  }

  template<class _SampleType>
  void write(long index) {

    auto& out = *_io.output_buffer;
    for (int channel = 0; channel < _num_outputs; ++channel) {
      const auto buffer = static_cast<__asio_sample<_SampleType>*>(_asio_buffers[_num_inputs + channel].buffers[index]);
      for (int frame = 0; frame < out.size_frames(); ++frame) {
        buffer[frame] = {out(frame, channel)};
      }
    }
  }

  static void sample_rate_changed(ASIOSampleRate) {}

  static long asio_message(long selector, long, void*, double*) {
    switch (selector) {
      case kAsioSupportsTimeInfo: return ASIOTrue;
      case kAsioEngineVersion: return 2;
      case kAsioSelectorSupported: return ASIOFalse;
      default: return ASIOFalse;
    }
  }

  static void buffer_switch(long index, ASIOBool) {
    instance()->on_buffer_switch(index);
  }

  void on_buffer_switch(long index) {

    _read(index);
    _user_callback(*this, _io);
    _write(index);

    _asio->outputReady();
  }

  static ASIOTime* buffer_switch_time_info(ASIOTime* time, long index, ASIOBool) {
    return instance()->on_buffer_switch(time, index);
  }

  ASIOTime* on_buffer_switch(ASIOTime* time, long index) {

    _read(index);
    _user_callback(*this, _io);
    _write(index);

    time->timeInfo.flags = kSystemTimeValid | kSamplePositionValid;
    time->timeInfo.samplePosition = _sample_position;
    _sample_position += _buffer_size;
    time->timeInfo.systemTime = timeGetTime() * 1'000'000;
    return time;
  }

  CComPtr<IASIO> _asio;
  const string _name;
  device_id_t _id;
  long _num_inputs{0};
  long _num_outputs{0};
  vector<sample_rate_t> _sample_rates;
  vector<buffer_size_t> _buffer_sizes;
  buffer_size_t _buffer_size{0};
  bool _running = false;

  ASIOSampleType _sample_type{};
  ASIOSamples _sample_position{0};

  using __asio_callback_t = function<void(audio_device&, audio_device_io<__asio_common_sample_t>&)>;
  __asio_callback_t _user_callback;
  audio_device_io<__asio_common_sample_t> _io;
  vector<__asio_common_sample_t> _input_samples;
  vector<__asio_common_sample_t> _output_samples;

  using fill_buffers = function<void(long index)>;
  fill_buffers _read;
  fill_buffers _write;

  vector<ASIOBufferInfo> _asio_buffers;
  ASIOCallbacks _asio_callbacks{};

  static audio_device* instance(optional<audio_device*> new_device = {}) {
    static audio_device* device = nullptr;
    if (new_device.has_value()) {
      device = *new_device;
    }
    return device;
  }
};

class audio_device_list : public forward_list<audio_device> {};

class __reg_key_reader final {
public:
  __reg_key_reader(HKEY key, const string& subkey) {
    RegOpenKeyExA(key, subkey.c_str(), 0, KEY_READ, &_key);
  }

  ~__reg_key_reader() {
    if (_key) {
      RegCloseKey(_key);
    }
  }

  vector<string> subkeys() const {
    constexpr int max_asio_name_length{32};
    char name[max_asio_name_length];
    vector<string> keys;

    for (DWORD index = 0;; ++index) {
      DWORD size{max_asio_name_length};
      const auto result = RegEnumKeyEx(_key, index, name, &size, nullptr, nullptr, nullptr, nullptr);

      if (result != ERROR_SUCCESS) {
        break;
      }

      keys.emplace_back(name);
    }

    return keys;
  }

  __reg_key_reader subkey(const string& name) const {
    return {_key, name};
  }

  string value(const char* name) const {
    DWORD size{0};
    RegQueryValueExA(_key, name, nullptr, nullptr, nullptr, &size);

    string value(size + 1, 0);
    RegQueryValueExA(_key, name, nullptr, nullptr, reinterpret_cast<LPBYTE>(value.data()), &size);

    return value;
  }

private:
  HKEY _key{nullptr};
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

  optional<audio_device> default_input() const {
    auto devices = inputs();
    if (devices.empty()) {
      return {};
    }
    return *devices.begin();
  }

  optional<audio_device> default_output() const {
    auto devices = outputs();
    if (devices.empty()) {
      return {};
    }
    return *devices.begin();
  }

  audio_device_list inputs() const {
    return enumerate(audio_direction::in);
  }

  audio_device_list outputs() const {
    return enumerate(audio_direction::out);
  }

private:
  __asio_devices() {
    CoInitialize(nullptr);
  }

  audio_device_list enumerate(audio_direction direction) const {
    audio_device_list devices;
    _index = 0;

    for (const auto& name : _asio_reg.subkeys()) {

      auto device = open_device(name, direction);

      if (device) {
        devices.push_front(move(*device));
      }
    }
    return devices;
  }

  optional<audio_device> open_device(const string& name, const audio_direction direction) const {

    if (is_excluded(name)) {
      return {};
    }
    const auto key = _asio_reg.subkey(name);
    const auto value = key.value("CLSID");

    CLSID class_id;
    CLSIDFromString(CComBSTR(value.c_str()), &class_id);

    CComPtr<IASIO> asio;
    const auto result = CoCreateInstance(class_id, nullptr, CLSCTX_INPROC_SERVER, class_id, reinterpret_cast<void**>(&asio));
    if (result) {
      throw audio_device_exception("Failed to open ASIO driver");
    }

    if (!is_connected(asio)) {
      return {};
    }

    const auto device = audio_device(_index++, name, asio, direction);
    if (!is_required(device, direction)) {
      return {};
    }

    return device;
  }

  static bool is_required(const audio_device& device, const audio_direction direction) {
    switch (direction) {
      case audio_direction::in: return device.is_input();
      case audio_direction::out: return device.is_output();
      case audio_direction::full_duplex: return device.is_input() && device.is_output();
      default: return false;
    }
  }

  static bool is_excluded(string_view name) {
    if (name == "Realtek ASIO") {
      // Realtek ASIO drivers seem unstable
      // Every other time is IASIO::createBuffers called, it fails with ASE_InvalidMode
      // Play continues after stop is called
      return true;
    }
    return false;
  }

  static bool is_connected(IASIO* asio) {
    return ASIOTrue == asio->init(nullptr);
  }

  __reg_key_reader _asio_reg{HKEY_LOCAL_MACHINE, R"(software\asio)"};
  mutable int _index{0};
};

_LIBSTDAUDIO_NAMESPACE_END
