// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include <__audio_device>

_LIBSTDAUDIO_NAMESPACE_BEGIN

class _device_impl {
public:
  _device_impl(device& owner)
    : _owner(owner) {
  }

  virtual ~_device_impl() = default;
  virtual string_view name() const = 0;
  virtual bool is_input() const noexcept = 0;
  virtual bool is_output() const noexcept = 0;
  virtual void connect(device::callback cb) = 0;
  virtual bool is_polling() const noexcept = 0;
  virtual void wait() = 0;
  virtual void process() = 0;

protected:
  device& _owner;
};

template <typename Impl, typename... Args>
device _make_device_with_impl(Args&&... args) {
  device new_device;
  new_device._impl = make_unique<Impl>(new_device, forward<Args>(args)...);
  return new_device;
}

_LIBSTDAUDIO_NAMESPACE_END