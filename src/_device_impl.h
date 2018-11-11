// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include "device.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

class _device_impl {
public:
  _device_impl(device& owner)
      : _owner(owner) {}

  virtual ~_device_impl() = default;

  virtual string_view name() const = 0;

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

LIBSTDAUDIO_NAMESPACE_END