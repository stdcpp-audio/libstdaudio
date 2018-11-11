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

    void process(device& owner) override {
      buffer_list empty_bl;
      if (_cb)
        invoke(_cb, owner, empty_bl);
    }

    bool is_input() const noexcept override {
      return false;
    }

    bool is_output() const noexcept override {
      return false;
    }
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

void device::connect(const device::callback& cb) {
  _impl->connect(cb);
}

void device::connect(device::callback&& cb) {
  _impl->connect(move(cb));
}

void device::process() {
  _impl->process(*(this));
}

device get_input_device() {
  return {};
}

device get_output_device() {
  return {};
}

LIBSTDAUDIO_NAMESPACE_END

