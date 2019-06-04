// libstdaudio
// Copyright (c) 2019 Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <array>

_LIBSTDAUDIO_NAMESPACE_BEGIN

class __asio_sample_int32_t
{
public:
  static constexpr double _scale = INT32_MAX;
  __asio_sample_int32_t() = default;
  __asio_sample_int32_t(int32_t value)
    : value{ value }
  {}
  __asio_sample_int32_t(float value)
    : __asio_sample_int32_t{ static_cast<int32_t>(value * _scale) }
  {}

  int32_t int_value() const {
    return value;
  }

private:
  int32_t value;
};

class alignas(1) __asio_sample_int24_t
{
public:
  static constexpr int32_t _scale = 0x7f'ffff;
  __asio_sample_int24_t() = default;
  __asio_sample_int24_t(int32_t value)
    : data{ value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff }
  {}
  __asio_sample_int24_t(float value)
    : __asio_sample_int24_t{ static_cast<int32_t>(value * _scale) }
  {}

  int32_t int_value() const {
    return ((data[2] << 24) >> 8) | ((data[1] << 8) & 0xff00) | (data[0] & 0xff);
  }

private:
  array<int8_t, 3> data;
};

_LIBSTDAUDIO_NAMESPACE_END
