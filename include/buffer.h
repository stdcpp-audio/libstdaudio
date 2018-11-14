// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include <cstddef>
#include "config.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

/** Represents different methods to order the samples inside a multi-channel
 *  audio buffer.
 */
enum class buffer_ordering : char {
  /** Samples that belong to the same audio frame are contiguous in memory. */
  interleaved,

  /** Samples that belong to the same audio channel are contiguous in memory. */
  deinterleaved
};

/** An audio buffer, providing a view into a range of multi-channel audio data. */
class buffer {
public:
  /** The numeric format used for a single sample. */
  using value_type = float;   // TODO: make this a template argument
  using size_type = size_t;

  /** Constructs a buffer object holding no audio data. */
  buffer() = default;

  /** Constructs a buffer object from a contiguous range of raw audio data.
   *  \param data The audio data.
   *  \param num_channels The number of channels.
   *  \param ordering The buffer ordering of the buffer.
   */
  buffer(span<value_type>, size_type num_channels, buffer_ordering ordering);

  /** Returns a view of the raw audio data. */
  span<value_type> raw() const noexcept { return _data; }

  /** Returns true if the audio channels are interleaved, false otherwise. */
  buffer_ordering get_ordering() const noexcept { return _ordering; }

private:
  span<value_type> _data;
  size_type _num_channels = 0;
  buffer_ordering _ordering = buffer_ordering::interleaved;
};

LIBSTDAUDIO_NAMESPACE_END