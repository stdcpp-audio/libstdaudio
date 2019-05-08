// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

_LIBSTDAUDIO_NAMESPACE_BEGIN

template <typename _SampleType>
class audio_buffer {
public:
  using index_type = size_t;

  // TODO: an audio_buffer should not be user-constructible
  audio_buffer(_SampleType* data, index_type data_size, index_type num_channels)
    : _samples(data, data_size), _num_channels(num_channels) {
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

  index_type size_bytes() const noexcept {
    return size_samples() * sizeof(_SampleType);
  }

  // TODO: replace samples() with something that does not require the data to be contiguous in memory.
  span<_SampleType> samples() const noexcept {
    return _samples;
  }

  _SampleType& operator()(index_type frame_index, index_type channel_index) {
    const index_type index = (_num_channels * frame_index) + channel_index;
    assert(index < _samples.size());
    return _samples[index];
  }

private:
  span<_SampleType> _samples = {};
  index_type _num_channels = 0;
};

template <typename _SampleType>
struct audio_device_io
{
  optional<audio_buffer<_SampleType>> input_buffer;
  optional<audio_buffer<_SampleType>> output_buffer;
};

_LIBSTDAUDIO_NAMESPACE_END
