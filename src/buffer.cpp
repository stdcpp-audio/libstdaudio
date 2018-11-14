// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "buffer.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

buffer::buffer(span<value_type> data, size_type num_channels, buffer_ordering ordering)
  : _data(data), _num_channels(num_channels), _ordering(ordering) {
}

LIBSTDAUDIO_NAMESPACE_END