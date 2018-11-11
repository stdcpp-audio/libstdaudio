// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "device.h"
#include "_device_impl.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

namespace {
  class _null_device_impl : public _device_impl {
  private:
    string_view name() const override {
      return {};
    }

    bool is_input() const noexcept override {
      return false;
    }

    bool is_output() const noexcept override {
      return false;
    }

    void connect(device::callback cb) override {
      _cb = move(cb);
    }

    bool is_polling() const noexcept override {
      // the null device does not have a callback mechanism.
      return true;
    }

    void wait() override {
    }

    void process(device& owner) override {
      // the current "process buffer" of the null device is always an empty buffer.
      // TODO: maybe this should be a no-op? What is more consistent?
      buffer_list empty_bl;
      if (_cb)
        invoke(_cb, owner, empty_bl);
    }

    device::callback _cb;
  };
}

device::device()
  : _impl{make_unique<_null_device_impl>()} {
}

device::device(device&&) noexcept = default;

device::~device() = default;

string_view device::name() const {
  return _impl->name();
}

bool device::is_input() const noexcept {
  return _impl->is_input();
}

bool device::is_output() const noexcept {
  return _impl->is_output();
}

void device::connect(device::callback cb) {
  _impl->connect(move(cb));
}

bool device::is_polling() const noexcept {
  return _impl->is_polling();
}

void device::wait() {
  _impl->wait();
}

void device::process() {
  _impl->process(*(this));
}

LIBSTDAUDIO_NAMESPACE_END

