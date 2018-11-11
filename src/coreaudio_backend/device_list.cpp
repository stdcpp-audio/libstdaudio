// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "device_list.h"

#ifdef LIBSTDAUDIO_BACKEND_COREAUDIO
#include <cctype>
#include <string>
#include <iostream>
#include <vector>
#include <AudioToolbox/AudioToolbox.h>
#include "../_device_impl.h"

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

  class _coreaudio_device_impl : public _device_impl {
  public:
    explicit _coreaudio_device_impl(string name)
      : _name(move(name)) {
    }

  private:
    void process(device& owner) override {
      // TODO: implement
    }

    string_view name() const override {
      return _name;
    }

    string _name;
  };

  class _coreaudio_device_enumerator {
  public:
    static _coreaudio_device_enumerator& get_instance() {
      static _coreaudio_device_enumerator cde;
      return cde;
    }

    auto get_input_device_list_impl() {
      forward_list<device> devices;
      const auto device_ids = _get_device_ids();

      for (const auto device_id : device_ids) {
        auto device_from_id = _get_device(device_id);
        // TODO: check it's an input device
        devices.push_front(move(device_from_id));
      }

      return devices;
    }

    auto get_output_device_list_impl() {
      forward_list<device> devices;
      const auto device_ids = _get_device_ids();

      for (const auto device_id : device_ids) {
        auto device_from_id = _get_device(device_id);
        // TODO: check it's an output device
        devices.push_front(move(device_from_id));
      }

      return devices;
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
      return _make_device_with_impl<_coreaudio_device_impl>(move(name));
    }

    static string _get_device_name(AudioDeviceID device_id) {
      AudioObjectPropertyAddress pa = {
        kAudioDevicePropertyDeviceName,
        kAudioObjectPropertyScopeWildcard,
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
  };
}

device_list& get_input_device_list() {
  auto& enumerator = _coreaudio_device_enumerator::get_instance();
  static device_list in_devices{enumerator.get_input_device_list_impl()};
  return in_devices;
}

device_list& get_output_device_list() {
  auto& enumerator = _coreaudio_device_enumerator::get_instance();
  static device_list out_devices{enumerator.get_output_device_list_impl()};
  return out_devices;
}

LIBSTDAUDIO_NAMESPACE_END
#endif // LIBSTDAUDIO_BACKEND_COREAUDIO