// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "buffer.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

buffer::channel_view::channel_view(const buffer &b, buffer::size_type num_channels)
  : _bptr(addressof(b)), _num_channels(num_channels) {
}

buffer::size_type buffer::channel_view::size() const noexcept {
  return _num_channels;
}

channel* buffer::channel_view::begin() const noexcept {
  // TODO: implement
  return nullptr;
}

channel* buffer::channel_view::end() const noexcept {
  // TODO: implement
  return nullptr;
}

buffer::buffer(span<value_type> data, size_type num_channels, buffer_ordering ordering)
  : _data(data), _ordering(ordering),
    _channel_view(*this, num_channels) {
}

LIBSTDAUDIO_NAMESPACE_END