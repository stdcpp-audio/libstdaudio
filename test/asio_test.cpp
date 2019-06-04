// libstdaudio
// Copyright (c) 2019 - Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include <array>
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE("Converts floating point samples to integer samples", "[asio]")
{

  SECTION("Converts to 32-bit samples")
  {
    CHECK(__asio_sample<int32_t>(0.f).int_value() == 0);
    CHECK(__asio_sample<int32_t>(1.f).int_value() == 0x7fff'ffff);
    CHECK(__asio_sample<int32_t>(-1.f).int_value() == -0x7fff'ffff);
    CHECK(__asio_sample<int32_t>(0.5f).int_value() == 0x7fff'ffff / 2);
    CHECK(__asio_sample<int32_t>(-0.5f).int_value() == -0x7fff'ffff / 2);
  }

  SECTION("Converts to 24-bit samples")
  {
    CHECK(__asio_sample_int24_t(0.f).int_value() == 0);
    CHECK(__asio_sample_int24_t(1.f).int_value() == 0x7f'ffff);
    CHECK(__asio_sample_int24_t(-1.f).int_value() == -0x7f'ffff);
    CHECK(__asio_sample_int24_t(0.5f).int_value() == 0x7f'ffff / 2);
    CHECK(__asio_sample_int24_t(-0.5f).int_value() == -0x7f'ffff / 2);
  }
}

