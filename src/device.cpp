// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <__audio_device>
#include "device_impl.h"

_LIBSTDAUDIO_NAMESPACE_BEGIN

namespace {
  class null_device_impl : public device_impl {
  public:
    null_device_impl(device& owner)
      : device_impl(owner) {
    }

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

    void connect(device::callback) override {
      // TODO: insert assertion. Connecting a callback to a null device is a no-op
    }

    bool is_polling() const noexcept override {
      // the null device does not have a callback mechanism.
      return true;
    }

    void wait() override {
    }

    void process() override {
    }
  };
}

device::device()
  : _impl{make_unique<null_device_impl>(*this)} {
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
  _impl->process();
}

_LIBSTDAUDIO_NAMESPACE_END

