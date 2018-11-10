// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once

#define LIBSTDAUDIO_NAMESPACE std::experimental::audio

#define LIBSTDAUDIO_NAMESPACE_BEGIN namespace LIBSTDAUDIO_NAMESPACE {
#define LIBSTDAUDIO_NAMESPACE_END   }

#ifdef __APPLE__
  #define LIBSTDAUDIO_BACKEND_COREAUDIO
#elif
  #define LIBSTDAUDIO_BACKEND_NONE
#endif