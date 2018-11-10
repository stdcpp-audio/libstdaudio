// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "audio.h"
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Device default construction", "[device]") {
  audio::device d;
  REQUIRE (true);
}

