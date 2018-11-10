// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "buffer_list.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

int buffer_list::num_input_buffers() const noexcept {
  return 0;
}

int buffer_list::num_output_buffers() const noexcept {
  return 0;
}

LIBSTDAUDIO_NAMESPACE_END