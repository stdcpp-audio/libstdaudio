// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <__audio_device>
#include "device_impl.h"

_LIBSTDAUDIO_NAMESPACE_BEGIN

namespace {
  class null_device_impl final : public device_impl {
  public:
    explicit null_device_impl(device& owner)
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

    buffer_order get_native_buffer_order() const noexcept override {
      // TODO: none of the two currently possible values make sense here!
      return {};
    }

    double get_sample_rate() const noexcept override {
      return 0;
    }

    size_t get_buffer_size_bytes() const noexcept override {
      return 0;
    }

    void start() override {
      throw device_exception{};
    }

    void stop() override {
      // no-op
    }

    bool is_running() const noexcept override {
      return false;
    }

    bool supports_callback() const noexcept override {
      return false;
    }

    bool supports_process() const noexcept override {
      return false;
    }

    void connect(device::callback) override {
      throw device_exception{};
    }

    void wait() const override {
      throw device_exception{};
    }

    void process(device::callback &c) override {
      throw device_exception{};
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

buffer_order device::get_native_order() const noexcept {
  return _impl->get_native_buffer_order();
}

double device::get_sample_rate() const noexcept {
  return _impl->get_sample_rate();
}

size_t device::get_buffer_size_bytes() const noexcept {
  return _impl->get_buffer_size_bytes();
}

void device::start() {
  _impl->start();
}

void device::stop() {
  _impl->stop();
}

bool device::is_running() const noexcept {
  return _impl->is_running();
}

bool device::supports_callback() const noexcept {
  return _impl->supports_callback();
}

bool device::supports_process() const noexcept {
  return _impl->supports_process();
}

void device::connect(device::callback cb) {
  _impl->connect(move(cb));
}

void device::wait() const {
  _impl->wait();
}

void device::process(callback& cb) {
  _impl->process(cb);
}

_LIBSTDAUDIO_NAMESPACE_END

