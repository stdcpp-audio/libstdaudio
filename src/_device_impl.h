// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include "device.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

class _device_impl {
public:
  virtual ~_device_impl() = default;

  virtual string_view name() const = 0;
  virtual bool is_input() const noexcept = 0;
  virtual bool is_output() const noexcept = 0;

  void connect(const device::callback& cb) {
    _cb = cb;
  }

  void connect(device::callback&& cb) {
    _cb = move(cb);
  }

  virtual void process(device& owner) = 0;

protected:
  device::callback _cb;
};

template <typename Impl, typename... Args>
device _make_device_with_impl(Args&&... args) {
  device new_device;
  new_device._impl = make_unique<Impl>(forward<Args>(args)...);
  return new_device;
}

LIBSTDAUDIO_NAMESPACE_END