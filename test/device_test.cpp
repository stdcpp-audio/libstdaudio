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
