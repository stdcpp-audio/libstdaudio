// libstdaudio
// Copyright (c) 2019 Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#include <array>
#include <numeric>

_LIBSTDAUDIO_NAMESPACE_BEGIN

template <typename _SampleType>
class __asio_sample
{
public:
  __asio_sample() = default;
  explicit __asio_sample(_SampleType value)
    : value{ value }
  {}
  explicit __asio_sample(float value)
    : __asio_sample(static_cast<_SampleType>(value * static_cast<double>(numeric_limits<_SampleType>::max())))
  {}

  int32_t int_value() const {
    return value;
  }

  float float_value() const {
    return static_cast<float>(value) / numeric_limits<_SampleType>::max();
  }

private:
  _SampleType value;
};

struct packed24_t{};

template <>
class alignas(1) __asio_sample<packed24_t>
{
public:
  static constexpr int32_t _scale = 0x7f'ffff;
  __asio_sample() = default;
  __asio_sample(int32_t value)
    : data{ value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff }
  {}
  __asio_sample(float value)
    : __asio_sample{ static_cast<int32_t>(value * _scale) }
  {}

  int32_t int_value() const {
    return ((data[2] << 24) >> 8) | ((data[1] << 8) & 0xff00) | (data[0] & 0xff);
  }

  float float_value() const {
    return static_cast<float>(int_value()) / _scale;
  }

private:
  array<int8_t, 3> data;
};

_LIBSTDAUDIO_NAMESPACE_END
