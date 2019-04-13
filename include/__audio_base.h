#pragma once

#include <optional>
#include <cassert>
#include <string_view>
#include <atomic>

// TODO: this is a temporary measure until std::span becomes available
#include "cpp/span.hpp"
using namespace TCB_SPAN_NAMESPACE_NAME;

#define _LIBSTDAUDIO_NAMESPACE std::experimental

#define _LIBSTDAUDIO_NAMESPACE_BEGIN namespace _LIBSTDAUDIO_NAMESPACE {
#define _LIBSTDAUDIO_NAMESPACE_END }

_LIBSTDAUDIO_NAMESPACE_BEGIN

struct audio_null_driver_t {};

#ifdef __APPLE__
  struct __coreaudio_driver_t {};
  using audio_default_driver_t = __coreaudio_driver_t;
#else

#if __has_include(<pulse/pulseaudio.h>)
  struct __pulseaudio_driver_t {};
#endif
#if __has_include(<portaudio.h>)
  struct __portaudio_driver_t {};
#endif

#if __has_include(<pulse/pulseaudio.h>)
  using audio_default_driver_t = __pulseaudio_driver_t;
#elif __has_include(<portaudio.h>)
  using audio_default_driver_t = __portaudio_driver_t;
#endif
#endif // __APPLE__

_LIBSTDAUDIO_NAMESPACE_END

