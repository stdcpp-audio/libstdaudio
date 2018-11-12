// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include <vector>
#include "config.h"
#include "buffer.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

/** A list of audio input or output buffers. */
class buffer_list {
public:
  // TODO: use span instead of vector when available
  vector<buffer> input_buffers;
  vector<buffer> output_buffers;
};

LIBSTDAUDIO_NAMESPACE_END