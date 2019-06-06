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
#include <map>
#include <AudioToolbox/AudioToolbox.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

// TODO: make __coreaudio_sample_type flexible according to the recommendation (see AudioSampleType).
using __coreaudio_native_sample_type = float;

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

  template <typename T, typename =  enable_if_t<is_unsigned_v<T>>>
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

class audio_device {
public:
  audio_device() = delete;

  ~audio_device() {
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
    if (find(_supported_buffer_sizes.begin(), _supported_buffer_sizes.end(), new_buffer_size) == _supported_buffer_sizes.end())
      return false;

    AudioObjectPropertyAddress pa = {
      kAudioDevicePropertyBufferFrameSize,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster
    };

    return __coreaudio_util::check_error(AudioObjectSetPropertyData(
      _device_id, &pa, 0, nullptr, sizeof(buffer_size_t), &new_buffer_size));
  }

  template <typename _SampleType>
  constexpr bool supports_sample_type() const noexcept {
    return is_same_v<_SampleType, __coreaudio_native_sample_type>;
  }

  constexpr bool can_connect() const noexcept {
    return true;
  }

  constexpr bool can_process() const noexcept {
    return false;
  }

  template <typename _CallbackType,
            typename = enable_if_t<is_nothrow_invocable_v<_CallbackType, audio_device&, audio_device_io<__coreaudio_native_sample_type >&>>>
  void connect(_CallbackType callback) {
    if (_running)
      throw audio_device_exception("cannot connect to running audio_device");

    _user_callback = move(callback);
  }

  // TODO: remove std::function as soon as C++20 default-ctable lambda and lambda in unevaluated contexts become available
  using no_op_t = std::function<void(audio_device&)>;

  template <typename _StartCallbackType = no_op_t,
            typename _StopCallbackType = no_op_t,
            // TODO: is_nothrow_invocable_t does not compile, temporarily replaced with is_invocable_t
            typename = enable_if_t<is_invocable_v<_StartCallbackType, audio_device&> && is_invocable_v<_StopCallbackType, audio_device&>>>
  bool start(_StartCallbackType&& start_callback = [](audio_device&) noexcept {},
             _StopCallbackType&& stop_callback = [](audio_device&) noexcept {}) {
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

  template <typename _CallbackType>
  void process(_CallbackType&) {
    assert(false);
  }

  constexpr bool has_unprocessed_io() const noexcept {
    return false;
  }

private:
  friend class __audio_device_enumerator;

  audio_device(device_id_t device_id, string name, __coreaudio_stream_config config)
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
                                   const AudioTimeStamp* /* now */,
                                   const AudioBufferList* input_data,
                                   const AudioTimeStamp* input_time,
                                   AudioBufferList* output_data,
                                   const AudioTimeStamp* output_time,
                                   void *void_ptr_to_this_device) {
    assert (void_ptr_to_this_device != nullptr);
    audio_device& this_device = *reinterpret_cast<audio_device*>(void_ptr_to_this_device);

    _fill_buffers(input_data, input_time, output_data, output_time, this_device._current_buffers);

    invoke(this_device._user_callback, this_device, this_device._current_buffers);
    return noErr;
  }

  static void _fill_buffers(const AudioBufferList* input_bl,
                            const AudioTimeStamp* input_time,
                            const AudioBufferList* output_bl,
                            const AudioTimeStamp* output_time,
                            audio_device_io<__coreaudio_native_sample_type>& buffers) {
    assert(input_bl != nullptr);
    assert(output_bl != nullptr);

    const size_t num_input_buffers = input_bl->mNumberBuffers;
    assert(num_input_buffers == 0 || num_input_buffers == 1);

    const size_t num_output_buffers = output_bl->mNumberBuffers;
    assert(num_output_buffers == 0 || num_output_buffers == 1);

    if (num_input_buffers == 1) {
      buffers.input_buffer = coreaudio_buffer_to_buffer(input_bl->mBuffers[0]);
      buffers.input_time = coreaudio_timestamp_to_timepoint (input_time);
    }

    if (num_output_buffers == 1) {
      buffers.output_buffer = coreaudio_buffer_to_buffer(output_bl->mBuffers[0]);
      buffers.output_time = coreaudio_timestamp_to_timepoint (output_time);
    }
  }

  static audio_buffer<__coreaudio_native_sample_type> coreaudio_buffer_to_buffer(const AudioBuffer& ca_buffer) {
    // TODO: allow different sample types here! It will possibly be int16_t instead of float on iOS!

    auto* data_ptr = reinterpret_cast<__coreaudio_native_sample_type*>(ca_buffer.mData);
    const size_t num_channels = ca_buffer.mNumberChannels;
    const size_t num_frames = ca_buffer.mDataByteSize / sizeof(__coreaudio_native_sample_type) / num_channels;

    return {data_ptr, num_frames, num_channels, contiguous_interleaved};
  }

  static audio_clock_t::time_point coreaudio_timestamp_to_timepoint(const AudioTimeStamp* timestamp) {
    auto count = static_cast<audio_clock_t::rep>(timestamp->mHostTime);
    return audio_clock_t::time_point() + audio_clock_t::duration(count);
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

  using __coreaudio_callback_t = function<void(audio_device&, audio_device_io<__coreaudio_native_sample_type>&)>;
  __coreaudio_callback_t _user_callback;
  audio_device_io<__coreaudio_native_sample_type> _current_buffers;
};

class audio_device_list : public forward_list<audio_device> {
};

class __audio_device_enumerator {
public:
  static __audio_device_enumerator& get_instance() {
    static __audio_device_enumerator cde;
    return cde;
  }

  optional<audio_device> get_default_io_device(AudioObjectPropertySelector selector) {
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
    audio_device_list devices;
    const auto device_ids = get_device_ids();

    for (const auto device_id : device_ids) {
      auto device_from_id = get_device(device_id);
      if (condition(device_from_id))
        devices.push_front(move(device_from_id));
    }

    return devices;
  }

  auto get_input_device_list() {
    return get_device_list([](const audio_device& d){
      return d.is_input();
    });
  }

  auto get_output_device_list() {
    return get_device_list([](const audio_device& d){
      return d.is_output();
    });
  }

private:
  __audio_device_enumerator() = default;

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

  static audio_device get_device(AudioDeviceID device_id) {
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

optional<audio_device> get_default_audio_input_device() {
  return __audio_device_enumerator::get_instance().get_default_io_device(
    kAudioHardwarePropertyDefaultInputDevice);
}

optional<audio_device> get_default_audio_output_device() {
  return __audio_device_enumerator::get_instance().get_default_io_device(
    kAudioHardwarePropertyDefaultOutputDevice);
}

audio_device_list get_audio_input_device_list() {
  return __audio_device_enumerator::get_instance().get_input_device_list();
}

audio_device_list get_audio_output_device_list() {
  return __audio_device_enumerator::get_instance().get_output_device_list();
}

struct __coreaudio_device_config_listener {
  static void register_callback(audio_device_list_event event, function<void()> cb) {
    static __coreaudio_device_config_listener dcl;
    const auto selector = get_coreaudio_selector(event);
    dcl.callbacks[selector] = move(cb);
  }

private:
  map<AudioObjectPropertySelector, function<void()>> callbacks;

  __coreaudio_device_config_listener() {
    coreaudio_add_internal_callback<kAudioHardwarePropertyDevices>();
    coreaudio_add_internal_callback<kAudioHardwarePropertyDefaultInputDevice>();
    coreaudio_add_internal_callback<kAudioHardwarePropertyDefaultOutputDevice>();
  }

  template <AudioObjectPropertySelector selector>
  void coreaudio_add_internal_callback() {
    AudioObjectPropertyAddress pa = {
        selector,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    if (!__coreaudio_util::check_error(AudioObjectAddPropertyListener(
        kAudioObjectSystemObject, &pa, &coreaudio_internal_callback<selector>, this))) {
      assert(false); // failed to register device config listener!
    }
  }

  template <AudioObjectPropertySelector selector>
  static OSStatus coreaudio_internal_callback(AudioObjectID device_id,
                                              UInt32 /* inNumberAddresses */,
                                              const AudioObjectPropertyAddress* /* inAddresses */,
                                              void* void_ptr_to_this_listener) {
    __coreaudio_device_config_listener& this_listener = *reinterpret_cast<__coreaudio_device_config_listener*>(void_ptr_to_this_listener);
    this_listener.call<selector>();
    return {};
  }

  template <AudioObjectPropertySelector selector>
  void call() {
    if (auto cb_iter = callbacks.find(selector); cb_iter != callbacks.end()) {
      invoke(cb_iter->second);
    }
  }

  static constexpr AudioObjectPropertySelector get_coreaudio_selector(audio_device_list_event event) noexcept {
    switch (event) {
      case audio_device_list_event::device_list_changed:
        return kAudioHardwarePropertyDevices;
      case audio_device_list_event::default_input_device_changed:
        return kAudioHardwarePropertyDefaultInputDevice;
      case audio_device_list_event::default_output_device_changed:
        return kAudioHardwarePropertyDefaultOutputDevice;
      default:
        assert(false); // invalid event!
        return {};
    }
  }
};

template <typename F, typename /* = enable_if_t<is_nothrow_invocable_v<F>> */ >
void set_audio_device_list_callback(audio_device_list_event event, F&& cb) {
  __coreaudio_device_config_listener::register_callback(event, function<void()>(cb));
}

_LIBSTDAUDIO_NAMESPACE_END
