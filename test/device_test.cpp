// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "catch/catch.hpp"
#include "audio.h"

using namespace std::experimental;

TEST_CASE( "Device default construction", "[device]") {
  audio::device d;
  REQUIRE (true);
}

