// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental::audio;

TEST_CASE( "Construct channel view from empty buffer", "[buffer_view]") {
  buffer b;
  channel_view channels(b);
  REQUIRE(channels.empty());
  REQUIRE(channels.size() == 0);
}

TEST_CASE( "Construct frame view from empty buffer", "[buffer_view]") {
  buffer b;
  frame_view frames(b);
  REQUIRE(frames.empty());
  REQUIRE(frames.size() == 0);
}
