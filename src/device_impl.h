// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include <__audio_device>

_LIBSTDAUDIO_NAMESPACE_BEGIN

class device_impl {
public:
  explicit device_impl(device& owner)
    : _owner(owner) {
  }

  virtual ~device_impl() = default;
  virtual string_view name() const = 0;
  virtual bool is_input() const noexcept = 0;
  virtual bool is_output() const noexcept = 0;
  virtual buffer_order get_native_ordering() const noexcept = 0;
  virtual double get_sample_rate() const noexcept = 0;
  virtual size_t get_buffer_size() const noexcept = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual bool is_running() const noexcept = 0;
  virtual bool supports_callback() const noexcept = 0;
  virtual bool supports_process() const noexcept = 0;
  virtual void connect(device::callback cb) = 0;
  virtual void wait() const = 0;
  virtual void process(device::callback& c) = 0;

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