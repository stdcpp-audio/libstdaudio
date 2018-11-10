// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "device.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

class _device_impl {
public:
  _device_impl(device& owner)
    : _owner(owner) {}

  virtual ~_device_impl() = default;

  void connect(device::callback& cb) {
    _cb = &cb;
  }

  virtual void process() = 0;

protected:
  device& _owner;
  device::callback* _cb {nullptr};
};

namespace {
  class _null_device_impl : public _device_impl {
  public:
    _null_device_impl(device& owner)
      : _device_impl(owner) {}

  private:
    void process() override {
      buffer_list empty_bl;
      if (_cb && *_cb)
        std::invoke(*_cb, _owner, empty_bl);
    }
  };
}

device::device()
  : _impl{new _null_device_impl{*this}} {
}

void device::connect(device::callback& cb) {
  _impl->connect(cb);
}

void device::process() {
  _impl->process();
}

device::~device() = default;

LIBSTDAUDIO_NAMESPACE_END

