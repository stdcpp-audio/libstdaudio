// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <memory>
#include "config.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

class _device_impl;

/** A handle to an audio input and/or output device. */
class device {
public:
  /** Default constructor.
   *  Constructs a null device, i.e. a device with no input and output channels.
   */
  device();

  /** Destructor. */
  ~device();

private:
  unique_ptr<_device_impl> _impl;
};

LIBSTDAUDIO_NAMESPACE_END