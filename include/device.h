// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include <memory>
#include <functional>
#include "config.h"
#include "buffer_list.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

class _device_impl;

/** A handle to an audio input and/or output device. */
class device {
public:
  /** Default constructor.
   *  Constructs a null device, i.e. a device with no input and output channels.
   */
  device();

  /** Move constructor. */
  device(device&&);

  /** Destructor. */
  ~device();

  /** Audio callback type. */
  using callback = std::function<void(device&, buffer_list&)>;

  /** Connect a callback to the device. If the device is polling, call device::process()
   *  to cause the device to call the callback. Otherwise, the callback will be
   *  automatically called on the audio thread. Any previously connected callbacks
   *  will be disconnected.
   *  This overload copies the callback into the device.
   */
  void connect(const callback&);

  /** Connect a callback to the device. If the device is polling, call device::process()
   *  to cause the device to call the callback. Otherwise, the callback will be
   *  automatically called on the audio thread. Any previously connected callbacks
   *  will be disconnected.
   *  This overload moves the callback into the device.
   */
  void connect(callback&&);

  /** Call the callback once on the current thread, passing in the current
   *  processing buffer.
   */
  void process();

private:
  unique_ptr<_device_impl> _impl;
};

/** Returns the current default audio input device. */
device get_input_device();

/** Returns the current default audio output device. */
device get_output_device();

LIBSTDAUDIO_NAMESPACE_END