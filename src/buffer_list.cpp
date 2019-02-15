// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <__audio_buffer_list>

_LIBSTDAUDIO_NAMESPACE_BEGIN

buffer_list::buffer_list() {
  _preallocate_safe_num_buffers();
}

buffer_list::buffer_list(size_type num_input_buffers, size_type num_output_buffers)
 : _input_buffers(num_input_buffers),
   _output_buffers(num_output_buffers) {
  _preallocate_safe_num_buffers();
}

void buffer_list::resize(size_type num_input_buffers, size_type num_output_buffers) {
  _input_buffers.resize(num_input_buffers);
  _output_buffers.resize(num_output_buffers);
}

span<buffer> buffer_list::input_buffers() {
  return _input_buffers;
}

span<buffer> buffer_list::output_buffers() {
  return _output_buffers;
}

void buffer_list::_preallocate_safe_num_buffers() {
  _input_buffers.reserve(_safe_num_buffers);
  _output_buffers.reserve(_safe_num_buffers);
}

size_t buffer_list::input_buffer_capacity() const noexcept {
  return _input_buffers.capacity();
}

size_t buffer_list::output_buffer_capacity() const noexcept {
  return _output_buffers.capacity();
}

_LIBSTDAUDIO_NAMESPACE_END