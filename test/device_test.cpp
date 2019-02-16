// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Null device name", "[device]") {
  audio::device d;
  REQUIRE(d.name().empty());
}

TEST_CASE( "Null device has no input/output", "[device]") {
  audio::device d;
  REQUIRE(!d.is_input());
  REQUIRE(!d.is_output());
}

TEST_CASE( "Null device has no sample rate or buffer size", "[device]") {
  audio::device d;
  REQUIRE(d.get_sample_rate() == 0);
  REQUIRE(d.get_buffer_size_bytes() == 0);
}

TEST_CASE( "Null device does no audio processing", "[device]") {
  audio::device d;
  REQUIRE(!d.is_running());
  REQUIRE_THROWS_AS(d.start(), audio::device_exception);
  REQUIRE(!d.is_running());
  REQUIRE_NOTHROW(d.stop());
  REQUIRE_NOTHROW(d.stop());
  REQUIRE(!d.is_running());

  REQUIRE(!d.supports_callback());
  REQUIRE(!d.supports_process());

  audio::device::callback cb = [](audio::device&, audio::buffer_list&) {
    REQUIRE(false);
  };

  REQUIRE_THROWS_AS(d.connect(cb), audio::device_exception);
  REQUIRE_THROWS_AS(d.wait(), audio::device_exception);
  REQUIRE_THROWS_AS(d.process(cb), audio::device_exception);
}