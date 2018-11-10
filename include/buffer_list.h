// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include "config.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

/** A list of audio input or output buffers. */
class buffer_list {
public:
  /** Returns the number of input buffers. */
  int num_input_buffers() const noexcept;

  /** Returns the number of output buffers. */
  int num_output_buffers() const noexcept;
};

LIBSTDAUDIO_NAMESPACE_END