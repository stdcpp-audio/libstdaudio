// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <cassert>

#include <chrono>

_LIBSTDAUDIO_NAMESPACE_BEGIN

struct contiguous_interleaved_t{};
inline constexpr contiguous_interleaved_t contiguous_interleaved;

struct contiguous_deinterleaved_t{};
inline constexpr contiguous_deinterleaved_t contiguous_deinterleaved;

struct ptr_to_ptr_deinterleaved_t{};
inline constexpr ptr_to_ptr_deinterleaved_t ptr_to_ptr_deinterleaved;

template <typename _SampleType>
class audio_buffer {
public:
  using sample_type = _SampleType;
  using index_type = size_t;

  audio_buffer(sample_type* data, index_type num_frames, index_type num_channels, contiguous_interleaved_t)
    : _num_frames(num_frames),
      _num_channels(num_channels),
      _stride(_num_channels),
      _is_contiguous(true) {
    assert (num_channels <= _max_num_channels);
    for (auto i = 0; i < _num_channels; ++i) {
      _channels[i] = data + i;
    }
  }

  audio_buffer(sample_type* data,  index_type num_frames, index_type num_channels, contiguous_deinterleaved_t)
      : _num_frames(num_frames),
        _num_channels(num_channels),
        _stride(1),
        _is_contiguous(true) {
    assert (num_channels <= _max_num_channels);
    for (auto i = 0; i < _num_channels; ++i) {
      _channels[i] = data + (i * _num_frames);
    }
  }

  audio_buffer(sample_type** data, index_type num_frames, index_type num_channels,ptr_to_ptr_deinterleaved_t)
      : _num_frames(num_frames),
        _num_channels(num_channels),
        _stride(1),
        _is_contiguous(false) {
    assert (num_channels <= _max_num_channels);
    copy (data, data + _num_channels, _channels.begin());
  }

  sample_type* data() const noexcept {
    return _is_contiguous ? _channels[0] : nullptr;
  }

  bool is_contiguous() const noexcept {
    return _is_contiguous;
  }

  bool frames_are_contiguous() const noexcept {
    return _stride == _num_channels;
  }

  bool channels_are_contiguous() const noexcept {
    return _stride == 1;
  }

  index_type size_frames() const noexcept {
    return _num_frames;
  }

  index_type size_channels() const noexcept {
    return _num_channels;
  }

  index_type size_samples() const noexcept {
    return _num_channels * _num_frames;
  }

  sample_type& operator()(index_type frame, index_type channel) noexcept {
    return const_cast<sample_type&>(as_const(*this).operator()(frame, channel));
  }

  const sample_type& operator()(index_type frame, index_type channel) const noexcept {
    return _channels[channel][frame * _stride];
  }

private:
  bool _is_contiguous = false;
  index_type _num_frames = 0;
  index_type _num_channels = 0;
  index_type _stride = 0;
  constexpr static size_t _max_num_channels = 16;
  std::array<sample_type*, _max_num_channels> _channels = {};
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
