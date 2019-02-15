// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <__audio_device_list>

#ifdef LIBSTDAUDIO_BACKEND_NONE
_LIBSTDAUDIO_NAMESPACE_BEGIN

// Dummy implementation for platforms that do not support any audio devices

device get_input_device() {
  return {};
}

device get_output_device() {
  return {};
}

device_list& get_input_device_list() {
  static device_list in_devices{device_list::_underlying_container{}};
  return in_devices;
}

device_list& get_output_device_list() {
  static device_list out_devices{device_list::_underlying_container{}};
  return out_devices;
}

int buffer_list::num_input_buffers() const noexcept {
  return 0;
}

int buffer_list::num_output_buffers() const noexcept {
  return 0;
}

_LIBSTDAUDIO_NAMESPACE_END
#endif // LIBSTDAUDIO_BACKEND_NONE