// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental::audio;

TEST_CASE( "Null device name", "[device]") {
  device d;
  REQUIRE(d.name().empty());
}

TEST_CASE( "Null device has no input/output", "[device]") {
  device d;
  REQUIRE(!d.is_input());
  REQUIRE(!d.is_output());
}

TEST_CASE( "Null device has no sample rate or buffer size", "[device]") {
  device d;
  REQUIRE(d.get_sample_rate() == 0);
  REQUIRE(d.get_buffer_size_bytes() == 0);
}

TEST_CASE( "Null device does no audio processing", "[device]") {
  device d;
  REQUIRE(!d.is_running());
  REQUIRE_THROWS_AS(d.start(), device_exception);
  REQUIRE(!d.is_running());
  REQUIRE_NOTHROW(d.stop());
  REQUIRE_NOTHROW(d.stop());
  REQUIRE(!d.is_running());

  REQUIRE(!d.supports_callback());
  REQUIRE(!d.supports_process());

  device::callback cb = [](device&, buffer_list&) {
    REQUIRE(false);
  };

  REQUIRE_THROWS_AS(d.connect(cb), device_exception);
  REQUIRE_THROWS_AS(d.wait(), device_exception);
  REQUIRE_THROWS_AS(d.process(cb), device_exception);
}