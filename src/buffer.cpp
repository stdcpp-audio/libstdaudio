// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <__audio_buffer>
#include <__audio_buffer_view>
#include <vector>

_LIBSTDAUDIO_NAMESPACE_BEGIN

buffer::buffer(span<sample_type> data, size_t num_channels, buffer_order ordering)
  : _data(data), _num_channels(num_channels), _order(ordering) {
  assert(data.size() % num_channels == 0);
}

channel_view buffer::channels() const noexcept {
  return {*this};
}

frame_view buffer::frames() const noexcept {
  return {*this};
}

_LIBSTDAUDIO_NAMESPACE_END