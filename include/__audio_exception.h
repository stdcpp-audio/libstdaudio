#pragma once
#include <__audio_base.h>
#include <stdexcept>

_LIBSTDAUDIO_NAMESPACE_BEGIN

struct audio_device_exception : public runtime_error {
  explicit audio_device_exception(const char* what)
    : runtime_error(what) {
  }
};

_LIBSTDAUDIO_NAMESPACE_END
