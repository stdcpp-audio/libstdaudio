// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include <cstddef>
#include "config.h"
#include "../src/_strided_dynamic_extent_span.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

/** An audio sample, representing the amplitude of an audio signal at some
 *  discrete point in time.
 */
using value_type = float;
// TODO: This cannot stay a typedef at compile time. We might need to use different
// ones in the same program, as some audio APIs might support multiple formats.


/** A channel, i.e. an audio signal that is digitally sampled using linear
 *  pulse-code modulation and represented as a contiguous sequence of audio samples.
 */
using channel = _strided_dynamic_extent_span<value_type>;


/** A frame, i.e. a contiguous sequence of audio samples, one per channel,
 *  that together represent the amplitudes of a set of audio channels at
 *  some discrete point in time.
 */
using frame = _strided_dynamic_extent_span<value_type>;


/** Represents different methods to order the samples inside a multi-channel
 *  audio buffer.
 */
enum class buffer_ordering : char {
  /** Samples that belong to the same audio frame are contiguous in memory. */
  interleaved,

  /** Samples that belong to the same audio channel are contiguous in memory. */
  deinterleaved
};


/** An audio buffer, providing a view into a range of multi-channel audio data.
 *  The buffer does not own the data. Therefore, constructing and copying a
 *  buffer does not involve memory allocations.
 */
class buffer {
public:
  /** The numeric format used for a single sample. */
  using value_type = float;   // TODO: make this a template argument?
  using size_type = size_t;

  /** Constructs a buffer object holding no audio data. */
  buffer() = default;

  /** Constructs a buffer object from a contiguous range of raw audio data.
   *  \param data The audio data.
   *  \param num_channels The number of channels.
   *  \param ordering The buffer ordering of the buffer.
   *
   *  Expects: data.size() is a multiple of num_channels.
   */
  buffer(span<value_type> data, size_type num_channels, buffer_ordering ordering);

  /** Returns a view of the raw audio data. */
  span<value_type> raw() const noexcept { return _data; }

  /** Returns true if the buffer holds no audio data, false otherwise. */
  bool empty() const noexcept { return _data.empty(); }

  /** Returns the size of the buffer audio data in bytes. */
  size_type size_bytes() const noexcept { return _data.size_bytes(); }

  /** Returns the ordering of the buffer audio data. */
  buffer_ordering get_ordering() const noexcept { return _ordering; }

  /** A view over the channels of an audio buffer. */
  using channel_view = _strided_dynamic_extent_span<value_type>;

  /** Returns a channel view of this buffer. */
  const channel_view& channels() const noexcept { return _channel_view; }

private:
  span<value_type> _data;
  buffer_ordering _ordering = buffer_ordering::interleaved;
  channel_view _channel_view;
};

LIBSTDAUDIO_NAMESPACE_END