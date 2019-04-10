// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

_LIBSTDAUDIO_NAMESPACE_BEGIN

template<>
class audio_basic_device<audio_null_driver_t>
{
public:
  audio_basic_device() = delete;
};

template<>
class audio_basic_device_list<audio_null_driver_t>
{
private:
  class iterator {
  public:
    auto operator==(const iterator&) const noexcept { return true; }
    auto operator!=(const iterator&) const noexcept { return false; }
    auto operator++() -> const iterator& { assert(false); return *this; }
    auto operator*() -> audio_basic_device<audio_null_driver_t>& { assert(false); }
  };

public:
  auto begin() -> iterator { return {}; }
  auto end() -> iterator { return {}; }
  auto empty() -> bool { return true; }
};

template<>
auto get_default_audio_input_device<audio_null_driver_t>()
  -> optional<audio_basic_device<audio_null_driver_t>>
{
  return {};
}

template<>
auto get_default_audio_output_device<audio_null_driver_t>()
  -> optional<audio_basic_device<audio_null_driver_t>>
{
  return {};
}

template<>
auto get_audio_input_device_list<audio_null_driver_t>()
  -> audio_basic_device_list<audio_null_driver_t>
{
  return {};
}

template<>
auto get_audio_output_device_list<audio_null_driver_t>()
  -> audio_basic_device_list<audio_null_driver_t>
{
  return {};
}

_LIBSTDAUDIO_NAMESPACE_END