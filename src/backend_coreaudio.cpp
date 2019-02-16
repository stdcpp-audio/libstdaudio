// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <__audio_device_list>

#ifdef LIBSTDAUDIO_BACKEND_COREAUDIO
#include <cctype>
#include <string>
#include <iostream>
#include <vector>
#include <AudioToolbox/AudioToolbox.h>
#include "device_impl.h"

_LIBSTDAUDIO_NAMESPACE_BEGIN

namespace {
  class coreaudio_util {
  public:
    static bool check_error(OSStatus error) {
      if (error == noErr)
        return true;

      _log_message(_format_error(error));
      return false;
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
      cerr << "coreaudio_backend error: " << s << endl;
    }
  };

  struct coreaudio_stream_config {
    AudioBufferList input_config = {0};
    AudioBufferList output_config = {0};
  };

  buffer coreaudio_buffer_to_buffer(const AudioBuffer &ca_buffer) {
    // TODO: allow different sample types here! It will possibly be int16_t instead of float on iOS!

    const size_t num_channels = ca_buffer.mNumberChannels;
    auto* data_ptr = reinterpret_cast<sample_type*>(ca_buffer.mData);
    const size_t data_size = ca_buffer.mDataByteSize / sizeof(sample_type);

    return {
      span<sample_type>{data_ptr, static_cast<span<sample_type>::index_type>(data_size)},
      static_cast<span<sample_type>::index_type>(num_channels),
      buffer_order::interleaved
    };
  }

  bool coreaudio_fill_buffer_list(const AudioBufferList *ca_input_bl,
                                  const AudioBufferList *ca_output_bl,
                                  buffer_list &bl) {
    assert(ca_input_bl != nullptr);
    assert(ca_output_bl != nullptr);

    const size_t num_input_buffers = ca_input_bl->mNumberBuffers;
    const size_t num_output_buffers = ca_output_bl->mNumberBuffers;

    if (bl.__input_buffer_capacity() < num_input_buffers ||
      bl.__output_buffer_capacity() < num_output_buffers) {
      // FAIL! we would have to allocate memory in order to process this data.
      // TODO: introduce some debug-mode-only assert macro for this kind of runtime error!
      // TODO: how do we tell the user about such errors? we can't throw exceptions in the audio thread
      return false;
    }

    bl.resize(num_input_buffers, num_output_buffers);

    for (size_t i_buf = 0; i_buf < num_input_buffers; ++i_buf) {
      const AudioBuffer& ca_buffer = ca_input_bl->mBuffers[i_buf];
      bl.input_buffers()[i_buf] = coreaudio_buffer_to_buffer(ca_buffer);
    }

    for (size_t i_buf = 0; i_buf < num_output_buffers; ++i_buf) {
      const AudioBuffer& ca_buffer = ca_output_bl->mBuffers[i_buf];
      bl.output_buffers()[i_buf] = coreaudio_buffer_to_buffer(ca_buffer);
    }

    return true;
  }

  class coreaudio_device_impl final : public device_impl {
  public:
    coreaudio_device_impl(device& owner,
                           AudioObjectID device_id,
                           string name,
                           coreaudio_stream_config config)
      : device_impl(owner),
        _device_id(device_id), _name(move(name)), _config(config) {
    }

    ~coreaudio_device_impl() override {
        stop();
    }

  private:
    string_view name() const override {
      return _name;
    }

    bool is_input() const noexcept override {
      return _config.input_config.mNumberBuffers != 0;
    }

    bool is_output() const noexcept override {
      return _config.output_config.mNumberBuffers != 0;
    }

    buffer_order get_native_buffer_order() const noexcept override {
      return buffer_order::interleaved;
    }

    double get_sample_rate() const noexcept override {
      AudioObjectPropertyAddress pa = {
        kAudioDevicePropertyNominalSampleRate,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
      };

      uint32_t data_size = 0;
      if (!coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
        _device_id, &pa, 0, nullptr, &data_size)))
        return {};

      if (data_size != sizeof(double))
        return {};

      double sample_rate = 0;

      if (!coreaudio_util::check_error(AudioObjectGetPropertyData(
        _device_id, &pa, 0, nullptr, &data_size, &sample_rate)))
        return {};

      return sample_rate;
    }

    size_t get_buffer_size() const noexcept override {
      AudioObjectPropertyAddress pa = {
        kAudioDevicePropertyBufferSize,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
      };

      uint32_t data_size = 0;
      if (!coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
        _device_id, &pa, 0, nullptr, &data_size)))
        return {};

      if (data_size != sizeof(uint32_t))
        return {};

      uint32_t buffer_size = 0;

      if (!coreaudio_util::check_error(AudioObjectGetPropertyData(
        _device_id, &pa, 0, nullptr, &data_size, &buffer_size)))
        return {};

      return buffer_size;
    }

    void start() override {
      if (!_running) {
        // TODO: ProcID is a resource; wrap it into an RAII guard
        if (!coreaudio_util::check_error(AudioDeviceCreateIOProcID(
            _device_id, device_callback, this, &_proc_id)))
          return;

        if (!coreaudio_util::check_error(AudioDeviceStart(
            _device_id, device_callback))) {
          coreaudio_util::check_error(AudioDeviceDestroyIOProcID(
            _device_id, _proc_id));

          _proc_id = {};
          return;
        }

        _running = true;
      }
    }

    void stop() override {
      if (_running) {
        coreaudio_util::check_error(AudioDeviceStop(
            _device_id, device_callback));

        coreaudio_util::check_error(AudioDeviceDestroyIOProcID(
          _device_id, _proc_id));

        _proc_id = {};
        _running = false;
      }
    }

    bool is_running() const noexcept override {
      return _running;
    }

    bool supports_callback() const noexcept override {
      return true;
    }

    bool supports_process() const noexcept override {
      return false;
    }

    void connect(device::callback cb) override {
      if (_running)
        throw device_exception{};

      _user_callback = move(cb);
    }

    void wait() const override {
      throw device_exception{};
    }

    void process(device::callback& c) override {
      throw device_exception{};
    }

    static OSStatus device_callback(AudioObjectID device_id,
                                    const AudioTimeStamp *,
                                    const AudioBufferList *input_data,
                                    const AudioTimeStamp *,
                                    AudioBufferList *output_data,
                                    const AudioTimeStamp *,
                                    void *void_ptr_to_this_device) {
      assert (void_ptr_to_this_device != nullptr);
      coreaudio_device_impl& this_device = *reinterpret_cast<coreaudio_device_impl*>(void_ptr_to_this_device);

      if (!coreaudio_fill_buffer_list(input_data, output_data, this_device._current_buffer_list))
        return noErr;  // TODO: Return some kind of error to CoreAudio? which?

      invoke(this_device._user_callback, this_device._owner, this_device._current_buffer_list);
      return noErr;
    }

    AudioObjectID _device_id = {};
    AudioDeviceIOProcID _proc_id = {};
    bool _running = false;
    string _name;
    coreaudio_stream_config _config;
    device::callback _user_callback;
    buffer_list _current_buffer_list;
  };

  class coreaudio_device_enumerator {
  public:
    static coreaudio_device_enumerator& get_instance() {
      static coreaudio_device_enumerator cde;
      return cde;
    }

    device get_default_io_device(AudioObjectPropertySelector selector) {
      AudioObjectPropertyAddress pa = {
        selector,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
      };

      AudioDeviceID device_id;
      uint32_t data_size = sizeof(device_id);

      if (!coreaudio_util::check_error(AudioObjectGetPropertyData (
        kAudioObjectSystemObject, &pa, 0, nullptr, &data_size, &device_id)))
        return {};

      return get_device(device_id);
    }

    template <typename Condition>
    auto get_device_list_impl(Condition condition) {
      forward_list<device> devices;
      const auto device_ids = get_device_ids();

      for (const auto device_id : device_ids) {
        auto device_from_id = get_device(device_id);
        if (condition(device_from_id))
          devices.push_front(move(device_from_id));
      }

      return devices;
    }

    auto get_input_device_list_impl() {
      return get_device_list_impl([](const device& d){
        return d.is_input();
      });
    }

    auto get_output_device_list_impl() {
      return get_device_list_impl([](const device& d){
        return d.is_output();
      });
    }

  private:
    coreaudio_device_enumerator() = default;

    static vector<AudioDeviceID> get_device_ids() {
      AudioObjectPropertyAddress pa = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
      };

      uint32_t data_size = 0;
      if (!coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
        kAudioObjectSystemObject, &pa,
        0, nullptr, &data_size)))
        return {};

      const auto device_count = static_cast<uint32_t>(data_size / sizeof(AudioDeviceID));
      if (device_count == 0)
        return {};

      vector<AudioDeviceID> device_ids(device_count);

      if (!coreaudio_util::check_error(AudioObjectGetPropertyData(
        kAudioObjectSystemObject, &pa,
        0, nullptr, &data_size, device_ids.data())))
        return {};

      return device_ids;
    }

    static device get_device(AudioDeviceID device_id) {
      string name = get_device_name(device_id);
      auto config = get_device_io_stream_config(device_id);

      return _make_device_with_impl<coreaudio_device_impl>(device_id, move(name), config);
    }

    static string get_device_name(AudioDeviceID device_id) {
      AudioObjectPropertyAddress pa = {
        kAudioDevicePropertyDeviceName,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
      };

      uint32_t data_size = 0;
      if (!coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
        device_id, &pa, 0, nullptr, &data_size)))
        return {};

      char name_buffer[data_size];
      if (!coreaudio_util::check_error(AudioObjectGetPropertyData(
        device_id, &pa, 0, nullptr, &data_size, name_buffer)))
        return {};

      return name_buffer;
    }

    static coreaudio_stream_config get_device_io_stream_config(AudioDeviceID device_id) {
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
      if (!coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
        device_id, &pa, 0, nullptr, &data_size)))
        return {0};

      if (data_size != sizeof(AudioBufferList))
        return {0};

      AudioBufferList stream_config;

      if (!coreaudio_util::check_error(AudioObjectGetPropertyData(
        device_id, &pa, 0, nullptr, &data_size, &stream_config)))
        return {0};

      return stream_config;
    }
  };
}

device get_default_input_device() {
  return coreaudio_device_enumerator::get_instance().get_default_io_device(
    kAudioHardwarePropertyDefaultInputDevice);
}

device get_default_output_device() {
  return coreaudio_device_enumerator::get_instance().get_default_io_device(
    kAudioHardwarePropertyDefaultOutputDevice);
}

device_list& get_input_device_list() {
  // TODO: static device_list is not good, it means it will never get updated
  auto& enumerator = coreaudio_device_enumerator::get_instance();
  static device_list in_devices{enumerator.get_input_device_list_impl()};
  return in_devices;
}

device_list& get_output_device_list() {
  // TODO: static device_list is not good, it means it will never get updated
  auto& enumerator = coreaudio_device_enumerator::get_instance();
  static device_list out_devices{enumerator.get_output_device_list_impl()};
  return out_devices;
}

_LIBSTDAUDIO_NAMESPACE_END
#endif // LIBSTDAUDIO_BACKEND_COREAUDIO