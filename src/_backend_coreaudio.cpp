// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "device_list.h"

#ifdef LIBSTDAUDIO_BACKEND_COREAUDIO
#include <cctype>
#include <string>
#include <iostream>
#include <vector>
#include <AudioToolbox/AudioToolbox.h>
#include "_device_impl.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

namespace {
  class _coreaudio_util {
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

  struct _coreaudio_stream_config {
    AudioBufferList input_config = {0};
    AudioBufferList output_config = {0};
  };

  class _coreaudio_device_impl : public _device_impl {
  public:
    explicit _coreaudio_device_impl(AudioObjectID device_id, string name, _coreaudio_stream_config config)
      : _device_id(device_id), _name(move(name)), _config(config) {
    }

private:
    bool is_input() const noexcept override {
      return _config.input_config.mNumberBuffers != 0;
    }

    bool is_output() const noexcept override {
      return _config.output_config.mNumberBuffers != 0;
    }

    void connect(device::callback cb) override {
      // TODO: what happens if you call this repeatedly?
      _user_callback = move(cb);

      if (!_running) {
        if (!_coreaudio_util::check_error(AudioDeviceCreateIOProcID(
          _device_id, _device_callback, nullptr, &_proc_id)))
          return;

        if (!_coreaudio_util::check_error(AudioDeviceStart(
          _device_id, _device_callback))) {
          _coreaudio_util::check_error(AudioDeviceDestroyIOProcID(
            _device_id, _proc_id));
          _proc_id = {};
        }

        _running = true;
      }
    }

    void process(device& owner) override {
      // TODO: pass in actual buffer list instead of empty one
      // TODO: what to do if the device is not running/there is no buffer?
      buffer_list bl;
      _user_callback(owner, bl);
    }

    string_view name() const override {
      return _name;
    }

    static OSStatus _device_callback(AudioObjectID           inDevice,
                                     const AudioTimeStamp*   inNow,
                                     const AudioBufferList*  inInputData,
                                     const AudioTimeStamp*   inInputTime,
                                     AudioBufferList*        outOutputData,
                                     const AudioTimeStamp*   inOutputTime,
                                     void* __nullable        inClientData) {
      // TODO: convert AudioBufferList into device::buffer_list and pass to _user_callback
    }

    AudioObjectID _device_id = {};
    AudioDeviceIOProcID _proc_id = {};
    bool _running = false;
    string _name;
    _coreaudio_stream_config _config;
    device::callback _user_callback;
  };

  class _coreaudio_device_enumerator {
  public:
    static _coreaudio_device_enumerator& get_instance() {
      static _coreaudio_device_enumerator cde;
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

      if (!_coreaudio_util::check_error(AudioObjectGetPropertyData (
        kAudioObjectSystemObject, &pa, 0, nullptr, &data_size, &device_id)))
        return {};

      return _get_device(device_id);
    }

    template <typename Condition>
    auto get_device_list_impl(Condition condition) {
      forward_list<device> devices;
      const auto device_ids = _get_device_ids();

      for (const auto device_id : device_ids) {
        auto device_from_id = _get_device(device_id);
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
    _coreaudio_device_enumerator() = default;

    static vector<AudioDeviceID> _get_device_ids() {
      AudioObjectPropertyAddress pa = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
      };

      uint32_t data_size = 0;
      if (!_coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
        kAudioObjectSystemObject, &pa,
        0, nullptr, &data_size)))
        return {};

      const auto device_count = static_cast<uint32_t>(data_size / sizeof(AudioDeviceID));
      if (device_count == 0)
        return {};

      vector<AudioDeviceID> device_ids(device_count);

      if (!_coreaudio_util::check_error(AudioObjectGetPropertyData(
        kAudioObjectSystemObject, &pa,
        0, nullptr, &data_size, device_ids.data())))
        return {};

      return device_ids;
    }

    static device _get_device(AudioDeviceID device_id) {
      string name = _get_device_name(device_id);
      auto config = _get_device_io_stream_config(device_id);

      return _make_device_with_impl<_coreaudio_device_impl>(device_id, move(name), config);
    }

    static string _get_device_name(AudioDeviceID device_id) {
      AudioObjectPropertyAddress pa = {
        kAudioDevicePropertyDeviceName,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
      };

      uint32_t data_size = 0;
      if (!_coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
        device_id, &pa, 0, nullptr, &data_size)))
        return {};

      char name_buffer[data_size];
      if (!_coreaudio_util::check_error(AudioObjectGetPropertyData(
        device_id, &pa, 0, nullptr, &data_size, name_buffer)))
        return {};

      return name_buffer;
    }

    static _coreaudio_stream_config _get_device_io_stream_config(AudioDeviceID device_id) {
      return {
        _get_device_stream_config(device_id, kAudioDevicePropertyScopeInput),
        _get_device_stream_config(device_id, kAudioDevicePropertyScopeOutput)
      };
    }

    static AudioBufferList _get_device_stream_config(AudioDeviceID device_id, AudioObjectPropertyScope scope) {
      AudioObjectPropertyAddress pa = {
        kAudioDevicePropertyStreamConfiguration,
        scope,
        kAudioObjectPropertyElementMaster
      };

      uint32_t data_size = 0;
      if (!_coreaudio_util::check_error(AudioObjectGetPropertyDataSize(
        device_id, &pa, 0, nullptr, &data_size)))
        return {0};

      if (data_size != sizeof(AudioBufferList))
        return {0};

      AudioBufferList stream_config;

      if (!_coreaudio_util::check_error(AudioObjectGetPropertyData(
        device_id, &pa, 0, nullptr, &data_size, &stream_config)))
        return {0};

      return stream_config;
    }
  };
}

device get_input_device() {
  return _coreaudio_device_enumerator::get_instance().get_default_io_device(
    kAudioHardwarePropertyDefaultInputDevice);
}

device get_output_device() {
  return _coreaudio_device_enumerator::get_instance().get_default_io_device(
    kAudioHardwarePropertyDefaultOutputDevice);
}

device_list& get_input_device_list() {
  // TODO: static device_list is not good, it means it will never get updated
  auto& enumerator = _coreaudio_device_enumerator::get_instance();
  static device_list in_devices{enumerator.get_input_device_list_impl()};
  return in_devices;
}

device_list& get_output_device_list() {
  // TODO: static device_list is not good, it means it will never get updated
  auto& enumerator = _coreaudio_device_enumerator::get_instance();
  static device_list out_devices{enumerator.get_output_device_list_impl()};
  return out_devices;
}

int buffer_list::num_input_buffers() const noexcept {
  // TODO: implement
  return 0;
}

int buffer_list::num_output_buffers() const noexcept {
  // TODO: implement
  return 0;
}

LIBSTDAUDIO_NAMESPACE_END
#endif // LIBSTDAUDIO_BACKEND_COREAUDIO