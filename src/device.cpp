// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "device.h"
#include "_device_impl.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

namespace {
  class _null_device_impl : public _device_impl {
  public:
    _null_device_impl(device& owner)
      : _device_impl(owner) {}

  private:
    void process() override {
      buffer_list empty_bl;
      if (_cb)
        std::invoke(_cb, _owner, empty_bl);
    }
  };
}

device::device()
  : _impl{new _null_device_impl{*this}} {
}

device::device(device&&) = default;

device::~device() = default;

void device::connect(const device::callback& cb) {
  _impl->connect(cb);
}

void device::connect(device::callback&& cb) {
  _impl->connect(std::move(cb));
}

void device::process() {
  _impl->process();
}

device get_input_device() {
  return {};
}

device get_output_device() {
  return {};
}

LIBSTDAUDIO_NAMESPACE_END

