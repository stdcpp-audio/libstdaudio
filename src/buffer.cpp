// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <__audio_buffer>
#include <vector>

_LIBSTDAUDIO_NAMESPACE_BEGIN


buffer::buffer(span<value_type> data, size_type num_channels, buffer_ordering ordering)
  : _data(data), _ordering(ordering),
    _channel_view(this->_data.data(), this->_data.size(), this->_data.size() / num_channels) {
  assert(data.size() % num_channels == 0);
}

_LIBSTDAUDIO_NAMESPACE_END