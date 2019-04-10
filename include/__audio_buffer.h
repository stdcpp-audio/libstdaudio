// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

_LIBSTDAUDIO_NAMESPACE_BEGIN

struct audio_buffer_order_interleaved {};
struct audio_buffer_order_deinterleaved {};

template <typename _SampleType, typename _BufferOrder>
class audio_basic_buffer {
public:
  audio_basic_buffer(_SampleType* data, size_t data_size, size_t num_channels)
    : _samples(data, data_size), _num_channels(num_channels) {
    // TODO: currently only interleaved is supported
    static_assert(is_same_v<_BufferOrder, audio_buffer_order_interleaved>);
  }

  size_t size_channels() const noexcept {
    return _num_channels;
  }

  size_t size_frames() const noexcept {
    return size_samples() / size_channels();
  }

  size_t size_samples() const noexcept {
    return _samples.size();
  }

  size_t size_bytes() const noexcept {
    return size_samples() * sizeof(_SampleType);
  }

  span<_SampleType> samples() const noexcept {
    return _samples;
  }

  _SampleType& operator()(size_t frame_index, size_t channel_index) {
    const size_t index = (_num_channels * frame_index) + channel_index;
    assert(index < _samples.size());
    return _samples[index];
  }

private:
  span<_SampleType> _samples = {};
  size_t _num_channels = 0;
};

template <typename _SampleType,typename _BufferOrder>
struct audio_basic_device_buffers
{
  optional<audio_basic_buffer<_SampleType, _BufferOrder>> __input_buffer;
  optional<audio_basic_buffer<_SampleType, _BufferOrder>> __output_buffer;

  auto input_buffer() const noexcept
    -> optional<audio_basic_buffer<_SampleType, _BufferOrder>> {
    return __input_buffer;
  }
  auto output_buffer() const noexcept
    -> optional<audio_basic_buffer<_SampleType, _BufferOrder>> {
    return __output_buffer;
  }
};

using audio_buffer = audio_basic_buffer<float, audio_buffer_order_interleaved>;
using audio_device_buffers = audio_basic_device_buffers<float, audio_buffer_order_interleaved>;

_LIBSTDAUDIO_NAMESPACE_END
