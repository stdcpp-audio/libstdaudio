// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once

#define LIBSTDAUDIO_NAMESPACE std::experimental::audio

#define LIBSTDAUDIO_NAMESPACE_BEGIN namespace LIBSTDAUDIO_NAMESPACE {
#define LIBSTDAUDIO_NAMESPACE_END   }

// TODO: this is a temporary measure until std::span becomes available
#include "../src/cpp20/span.hpp"
using namespace TCB_SPAN_NAMESPACE_NAME;