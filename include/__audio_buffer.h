// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <cassert>

#include <chrono>

_LIBSTDAUDIO_NAMESPACE_BEGIN

// TODO: the audio_buffer currently only supports contiguous-interleaved. Make this generic using Gasper Azman's implementation!

template <typename _SampleType>
class audio_buffer {
public:
  using sample_type = _SampleType;
  using index_type = size_t;

  sample_type& operator()(index_type frame_index, index_type channel_index) {
    return _samples[_get_1d_index(frame_index, channel_index)];
  }

  const sample_type& operator()(index_type frame_index, index_type channel_index) const {
    return _samples[_get_1d_index(frame_index, channel_index)];
  }

  sample_type* data() const noexcept {
    return _samples.data();
  }

  constexpr bool is_contiguous() const noexcept {
    return true;
  }

  constexpr bool channels_are_contiguous() const noexcept {
    return false;
  }

  constexpr bool frames_are_contiguous() const noexcept {
    return true;
  }

  index_type size_channels() const noexcept {
    return _num_channels;
  }

  index_type size_frames() const noexcept {
    return size_samples() / size_channels();
  }

  index_type size_samples() const noexcept {
    return _samples.size();
  }

private:
  index_type _get_1d_index(index_type frame_index, index_type channel_index) const {
    const index_type index = (_num_channels * frame_index) + channel_index;
    assert(index < size_t(_samples.size()));
    return index;
  }

  friend class audio_device;

  audio_buffer(_SampleType* data, index_type data_size, index_type num_channels)
      : _samples(data, data_size), _num_channels(num_channels) {
  }

  span<_SampleType> _samples = {};
  index_type _num_channels = 0;
};

// TODO: this is currently macOS specific!
using audio_clock_t = chrono::steady_clock;

template <typename _SampleType>
struct audio_device_io
{
  optional<audio_buffer<_SampleType>> input_buffer;
  optional<chrono::time_point<audio_clock_t>> input_time;
  optional<audio_buffer<_SampleType>> output_buffer;
  optional<chrono::time_point<audio_clock_t>> output_time;
};

_LIBSTDAUDIO_NAMESPACE_END
