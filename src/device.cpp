// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "device.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

class _device_impl {};

namespace {
  class _null_device_impl : public _device_impl {
  };
}

device::device()
  : _impl{new _null_device_impl} {
}

device::~device() = default;

LIBSTDAUDIO_NAMESPACE_END

