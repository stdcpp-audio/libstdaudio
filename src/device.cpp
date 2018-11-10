// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "device.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

class _device_impl {
public:
  _device_impl(device& owner)
    : _owner(owner) {}

  virtual ~_device_impl() = default;

  void connect(const device::callback& cb) {
    _cb = cb;
  }

  void connect(device::callback&& cb) {
    _cb = std::move(cb);
  }

  virtual void process() = 0;

protected:
  device& _owner;
  device::callback _cb;
};

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

void device::connect(const device::callback& cb) {
  _impl->connect(cb);
}

void device::connect(device::callback&& cb) {
  _impl->connect(std::move(cb));
}

void device::process() {
  _impl->process();
}

device::~device() = default;

device get_input_device() {
  return {};
}

device get_output_device() {
  return {};
}

LIBSTDAUDIO_NAMESPACE_END

