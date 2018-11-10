// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "config.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

/** Represents a handle to an audio input and/or output device. */
class device {
public:
  /** Default constructor.
   *  Constructs a null device, i.e. a device with no input and output channels.
   */
  device();
};

LIBSTDAUDIO_NAMESPACE_END