// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <cctype>
#include <string>
#include <iostream>
#include <vector>
#include <forward_list>
#include <AudioToolbox/AudioToolbox.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

// TODO: make __coreaudio_sample_type flexible according to the recommendation (see AudioSampleType).
using __coreaudio_native_sample_type = float;
using __coreaudio_native_buffer_order = audio_buffer_order_interleaved;

struct __coreaudio_stream_config {
  AudioBufferList input_config = {0};
  AudioBufferList output_config = {0};
};

class __coreaudio_util {
public:
  static bool check_error(OSStatus error) {
    if (error == noErr)
      return true;

    _log_message(_format_error(error));
    return false;
  }

  template <typename T, typename =  enable_if<is_unsigned_v<T>>>
  constexpr static T next_power_of_2(T n) noexcept {
    T count = 0;
    if (n && !(n & (n - 1)))
      return n;

    while (n != 0) {
      n >>= 1;
      count += 1;
    }

    return 1 << count;
  }

private:
  static string _format_error(OSStatus error)
  {
    char c[4];
    *(uint32_t *)(c) = CFSwapInt32HostToBig(uint32_t(error));
    if (_is_four_character_code(c))
      return string{"\'"} + c + "\'";

    return to_string(error);
  }

  static bool _is_four_character_code(char* c) {
    for (int i : {0, 1, 2, 3})
      if (!isprint(c[i]))
        return false;

    return true;
  }

  static void _log_message(const string& s) {
    // TODO: only do this in DEBUG
    cerr << "__coreaudio_backend error: " << s << endl;
  }
};

struct audio_device_exception : public runtime_error {
  explicit audio_device_exception(const char* what)
    : runtime_error(what) {
  }
};

using __coreaudio_device = audio_basic_device<__coreaudio_driver_t>;

template<>
class audio_basic_device<__coreaudio_driver_t>
{
public:
  audio_basic_device() = delete;

  ~audio_basic_device() {
    stop();
  }

  string_view name() const noexcept {
    return _name;
  }

  using device_id_t = AudioObjectID;

  device_id_t device_id() const noexcept {
    return _device_id;
  }

  bool is_input() const noexcept {
    return _config.input_config.mNumberBuffers == 1;
  }

  bool is_output() const noexcept {
    return _config.output_config.mNumberBuffers == 1;
  }

  int get_num_input_channels() const noexcept {
    return is_input() ? _config.input_config.mBuffers[0].mNumberChannels : 0;
  }

  int get_num_output_channels() const noexcept {
    return is_output() ? _config.output_config.mBuffers[0].mNumberChannels : 0;
  }

  using sample_rate_t = double;

  sample_rate_t get_sample_rate() const noexcept {
    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyNominalSampleRate,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    uint32_t data_size = 0;
    if (!__coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
      _device_id, &pa, 0, nullptr, &data_size)))
      return {};

    if (data_size != sizeof(double))
      return {};

    double sample_rate = 0;

    if (!__coreaudio_util::check_error(AudioObjectGetPropertyData(
      _device_id, &pa, 0, nullptr, &data_size, &sample_rate)))
      return {};

    return sample_rate;
  }

  span<const sample_rate_t> get_supported_sample_rates() const noexcept {
    assert(!_supported_sample_rates.empty());
    return {
      _supported_sample_rates.data(),
      static_cast<span<buffer_size_t>::index_type > (_supported_sample_rates.size())
      // TODO: remove the cast when you move on to a std::span implementation using index_type = size_t!
    };
  }

  bool set_sample_rate(sample_rate_t new_sample_rate) {
    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyNominalSampleRate,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    return __coreaudio_util::check_error(AudioObjectSetPropertyData(
      _device_id, &pa, 0, nullptr, sizeof(sample_rate_t), &new_sample_rate));
  }

  using buffer_size_t = uint32_t;

  buffer_size_t get_buffer_size_frames() const noexcept {
    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyBufferFrameSize,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    uint32_t data_size = 0;
    if (!__coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
      _device_id, &pa, 0, nullptr, &data_size)))
      return {};

    if (data_size != sizeof(buffer_size_t))
      return {};

    uint32_t buffer_size_frames = 0;

    if (!__coreaudio_util::check_error(AudioObjectGetPropertyData(
      _device_id, &pa, 0, nullptr, &data_size, &buffer_size_frames)))
      return {};

    return buffer_size_frames;
  }

  span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept {
    assert(!_supported_buffer_sizes.empty());
    return {
      _supported_buffer_sizes.data(),
      static_cast<span<buffer_size_t>::index_type > (_supported_buffer_sizes.size())
      // TODO: remove the cast when you move on to a std::span implementation using index_type = size_t!
    };
  }

  bool set_buffer_size_frames(buffer_size_t new_buffer_size) {
    // TODO: for some reason, the call below succeeds even for nonsensical buffer sizes, so we need to catch this  manually:
    if (std::find(_supported_buffer_sizes.begin(), _supported_buffer_sizes.end(), new_buffer_size) == _supported_buffer_sizes.end())
      return false;

    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyBufferFrameSize,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    return __coreaudio_util::check_error(AudioObjectSetPropertyData(
      _device_id, &pa, 0, nullptr, sizeof(buffer_size_t), &new_buffer_size));
  }

  template <typename _SampleType, typename _BufferOrder>
  constexpr bool supports_audio_buffer_type() const noexcept {
    return is_same_v<_SampleType, __coreaudio_native_sample_type>
      && is_same_v<_BufferOrder, __coreaudio_native_buffer_order>;
  }

  constexpr bool can_connect() const noexcept {
    return true;
  }

  constexpr bool can_process() const noexcept {
    return false;
  }

  template <typename _CallbackType>
  void connect(_CallbackType callback) {
    if (_running)
      throw audio_device_exception("cannot connect to running audio_device");

    _user_callback = move(callback);
  }

  bool start() {
    if (!_running) {
      // TODO: ProcID is a resource; wrap it into an RAII guard
      if (!__coreaudio_util::check_error(AudioDeviceCreateIOProcID(
          _device_id, _device_callback, this, &_proc_id)))
        return false;

      if (!__coreaudio_util::check_error(AudioDeviceStart(
          _device_id, _device_callback))) {
        __coreaudio_util::check_error(AudioDeviceDestroyIOProcID(
          _device_id, _proc_id));

        _proc_id = {};
        return false;
      }

      _running = true;
    }

    return true;
  }

  bool stop() {
    if (_running) {
      if (!__coreaudio_util::check_error(AudioDeviceStop(
        _device_id, _device_callback)))
        return false;

      if (!__coreaudio_util::check_error(AudioDeviceDestroyIOProcID(
        _device_id, _proc_id)))
        return false;

      _proc_id = {};
      _running = false;
    }

    return true;
  }

  bool is_running() const noexcept  {
    return _running;
  }

  void wait() const {
    assert(false);
  }

  template<class _Rep, class _Period>
  void wait_for(std::chrono::duration<_Rep, _Period> rel_time) const {
    assert(false);
  }

  template<class _Clock, class _Duration>
  void wait_until(std::chrono::time_point<_Clock, _Duration> abs_time) const {
    assert(false);
  }

  template <typename _CallbackType>
  void process(_CallbackType&) {
    assert(false);
  }

private:
  friend class __coreaudio_device_enumerator;

  audio_basic_device(device_id_t device_id, string name, __coreaudio_stream_config config)
  : _device_id(device_id),
    _name(move(name)),
    _config(config) {
    assert(!_name.empty());
    assert(config.input_config.mNumberBuffers == 0 || config.input_config.mNumberBuffers == 1);
    assert(config.output_config.mNumberBuffers == 0 || config.output_config.mNumberBuffers == 1);

    _init_supported_sample_rates();
    _init_supported_buffer_sizes();
  }

  static OSStatus _device_callback(AudioObjectID device_id,
                                   const AudioTimeStamp *,
                                   const AudioBufferList *input_data,
                                   const AudioTimeStamp *,
                                   AudioBufferList *output_data,
                                   const AudioTimeStamp *,
                                   void *void_ptr_to_this_device) {
    assert (void_ptr_to_this_device != nullptr);
    __coreaudio_device& this_device = *reinterpret_cast<__coreaudio_device*>(void_ptr_to_this_device);

    _fill_buffers(input_data, output_data, this_device._current_buffers);

    invoke(this_device._user_callback, this_device, this_device._current_buffers);
    return noErr;
  }

  static void _fill_buffers(const AudioBufferList* input_bl,
                            const AudioBufferList* output_bl,
                            audio_device_buffers& buffers) {
    assert(input_bl != nullptr);
    assert(output_bl != nullptr);

    const size_t num_input_buffers = input_bl->mNumberBuffers;
    assert(num_input_buffers == 0 || num_input_buffers == 1);

    const size_t num_output_buffers = output_bl->mNumberBuffers;
    assert(num_output_buffers == 0 || num_output_buffers == 1);

    if (num_input_buffers == 1)
      buffers.__input_buffer = coreaudio_buffer_to_buffer(input_bl->mBuffers[0]);

    if (num_output_buffers == 1)
      buffers.__output_buffer = coreaudio_buffer_to_buffer(output_bl->mBuffers[0]);
  }

  static audio_buffer coreaudio_buffer_to_buffer(const AudioBuffer& ca_buffer) {
    // TODO: allow different sample types here! It will possibly be int16_t instead of float on iOS!

    const size_t num_channels = ca_buffer.mNumberChannels;
    auto* data_ptr = reinterpret_cast<__coreaudio_native_sample_type*>(ca_buffer.mData);
    const size_t data_size = ca_buffer.mDataByteSize / sizeof(__coreaudio_native_sample_type);

    return {data_ptr, data_size, num_channels};
  }


  void _init_supported_sample_rates() {
    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyAvailableNominalSampleRates,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    uint32_t data_size = 0;
    if (!__coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
      _device_id, &pa, 0, nullptr, &data_size)))
      return;

    const size_t num_values = data_size / sizeof(AudioValueRange);
    AudioValueRange values[num_values];

    if (!__coreaudio_util::check_error(AudioObjectGetPropertyData(
      _device_id, &pa, 0, nullptr, &data_size, &values)))
      return;

    _supported_sample_rates.reserve(num_values);
    for (size_t i = 0; i < num_values; ++i) {
      assert(values[i].mMinimum == values[i].mMaximum);
      _supported_sample_rates.push_back(values[i].mMinimum);
    }
  }

  void _init_supported_buffer_sizes() {
    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyBufferFrameSizeRange,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    uint32_t data_size = 0;
    if (!__coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
      _device_id, &pa, 0, nullptr, &data_size)))
      return;

    if (data_size != sizeof(AudioValueRange)) {
      assert(false);
      return;
    }

    AudioValueRange buffer_size_range = {};

    if (!__coreaudio_util::check_error(AudioObjectGetPropertyData(
      _device_id, &pa, 0, nullptr, &data_size, &buffer_size_range))) {
      assert(false);
      return;
    }

    const auto min_buffer_size = static_cast<buffer_size_t>(buffer_size_range.mMinimum);
    const auto max_buffer_size = static_cast<buffer_size_t>(buffer_size_range.mMaximum);

    for (buffer_size_t buffer_size = __coreaudio_util::next_power_of_2(min_buffer_size); buffer_size <= max_buffer_size; buffer_size *= 2) {
      _supported_buffer_sizes.push_back(buffer_size);
    }
  }

  AudioObjectID _device_id = {};
  AudioDeviceIOProcID _proc_id = {};
  bool _running = false;
  string _name = {};
  __coreaudio_stream_config _config;
  vector<sample_rate_t> _supported_sample_rates;
  vector<buffer_size_t> _supported_buffer_sizes;

  using __coreaudio_callback_t = function<void(__coreaudio_device&, audio_device_buffers&)>;
  __coreaudio_callback_t _user_callback;
  audio_device_buffers _current_buffers;
};

template<>
class audio_basic_device_list<__coreaudio_driver_t> : public forward_list<__coreaudio_device> {
};

using __coreaudio_device_list = audio_basic_device_list<__coreaudio_driver_t>;

class __coreaudio_device_enumerator {
public:
  static __coreaudio_device_enumerator& get_instance() {
    static __coreaudio_device_enumerator cde;
    return cde;
  }

  optional<__coreaudio_device> get_default_io_device(AudioObjectPropertySelector selector) {
    AudioObjectPropertyAddress pa = {
      selector,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    AudioDeviceID device_id;
    uint32_t data_size = sizeof(device_id);

    if (!__coreaudio_util::check_error(AudioObjectGetPropertyData (
      kAudioObjectSystemObject, &pa, 0, nullptr, &data_size, &device_id)))
      return {};

    return get_device(device_id);
  }

  template <typename Condition>
  auto get_device_list(Condition condition) {
    __coreaudio_device_list devices;
    const auto device_ids = get_device_ids();

    for (const auto device_id : device_ids) {
      auto device_from_id = get_device(device_id);
      if (condition(device_from_id))
        devices.push_front(move(device_from_id));
    }

    return devices;
  }

  auto get_input_device_list() {
    return get_device_list([](const __coreaudio_device& d){
      return d.is_input();
    });
  }

  auto get_output_device_list() {
    return get_device_list([](const __coreaudio_device& d){
      return d.is_output();
    });
  }

private:
  __coreaudio_device_enumerator() = default;

  static vector<AudioDeviceID> get_device_ids() {
    AudioObjectPropertyAddress pa = {
      kAudioHardwarePropertyDevices,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    uint32_t data_size = 0;
    if (!__coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
      kAudioObjectSystemObject, &pa,
      0, nullptr, &data_size)))
      return {};

    const auto device_count = static_cast<uint32_t>(data_size / sizeof(AudioDeviceID));
    if (device_count == 0)
      return {};

    vector<AudioDeviceID> device_ids(device_count);

    if (!__coreaudio_util::check_error(AudioObjectGetPropertyData(
      kAudioObjectSystemObject, &pa,
      0, nullptr, &data_size, device_ids.data())))
      return {};

    return device_ids;
  }

  static __coreaudio_device get_device(AudioDeviceID device_id) {
    string name = get_device_name(device_id);
    auto config = get_device_io_stream_config(device_id);

    return {device_id, move(name), config};
  }

  static string get_device_name(AudioDeviceID device_id) {
    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyDeviceName,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    uint32_t data_size = 0;
    if (!__coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
      device_id, &pa, 0, nullptr, &data_size)))
      return {};

    char name_buffer[data_size];
    if (!__coreaudio_util::check_error(AudioObjectGetPropertyData(
      device_id, &pa, 0, nullptr, &data_size, name_buffer)))
      return {};

    return name_buffer;
  }

  static __coreaudio_stream_config get_device_io_stream_config(AudioDeviceID device_id) {
    return {
      get_device_stream_config(device_id, kAudioDevicePropertyScopeInput),
      get_device_stream_config(device_id, kAudioDevicePropertyScopeOutput)
    };
  }

  static AudioBufferList get_device_stream_config(AudioDeviceID device_id, AudioObjectPropertyScope scope) {
    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyStreamConfiguration,
      scope,
      kAudioObjectPropertyElementMaster
    };

    uint32_t data_size = 0;
    if (!__coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
      device_id, &pa, 0, nullptr, &data_size)))
      return {0};

    if (data_size != sizeof(AudioBufferList))
      return {0};

    AudioBufferList stream_config;

    if (!__coreaudio_util::check_error(AudioObjectGetPropertyData(
      device_id, &pa, 0, nullptr, &data_size, &stream_config)))
      return {0};

    return stream_config;
  }
};

template<>
auto get_default_audio_input_device<__coreaudio_driver_t>()
  -> optional<audio_basic_device<__coreaudio_driver_t>>
{
  return __coreaudio_device_enumerator::get_instance().get_default_io_device(
    kAudioHardwarePropertyDefaultInputDevice);
}

template<>
auto get_default_audio_output_device<__coreaudio_driver_t>()
  -> optional<audio_basic_device<__coreaudio_driver_t>>
{
  return __coreaudio_device_enumerator::get_instance().get_default_io_device(
    kAudioHardwarePropertyDefaultOutputDevice);
}

template<>
auto get_audio_input_device_list<__coreaudio_driver_t>()
  -> audio_basic_device_list<__coreaudio_driver_t>
{
  return __coreaudio_device_enumerator::get_instance().get_input_device_list();
}

template<>
auto get_audio_output_device_list<__coreaudio_driver_t>()
  -> audio_basic_device_list<__coreaudio_driver_t>
{
  return __coreaudio_device_enumerator::get_instance().get_output_device_list();
}

_LIBSTDAUDIO_NAMESPACE_END
