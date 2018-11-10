// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "device_list.h"
#include "../_device_impl.h"

#ifdef LIBSTDAUDIO_BACKEND_COREAUDIO
LIBSTDAUDIO_NAMESPACE_BEGIN

namespace {
  class _coreaudio_device_impl : public _device_impl {
  public:
    _coreaudio_device_impl(device& owner)
    : _device_impl(owner) {}

  private:
    void process() override {
      // TODO: implement
    }
  };
}

device_list& get_input_device_list() {
  // TODO: implement
  static device_list in_devices{device_list::_underlying_container{}};
  return in_devices;
}

device_list& get_output_device_list() {
  // TODO: implement
  static device_list out_devices{device_list::_underlying_container{}};
  return out_devices;
}

LIBSTDAUDIO_NAMESPACE_END
#endif // LIBSTDAUDIO_BACKEND_COREAUDIO