// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental::audio;

TEST_CASE( "Empty channel view size", "[buffer_view]") {
  buffer b;
  channel_view channels = b.channels();
  REQUIRE(channels.empty());
  REQUIRE(channels.size() == 0);
}

TEST_CASE( "Empty frame view size", "[buffer_view]") {
  buffer b;
  frame_view frames = b.frames();
  REQUIRE(frames.empty());
  REQUIRE(frames.size() == 0);
}

TEST_CASE( "Single-channel interleaved channel view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5};
  auto buf = buffer(data, 1, buffer_order::interleaved);

  auto channels = buf.channels();
  REQUIRE(!channels.empty());
  REQUIRE(channels.size() == 1);
}

TEST_CASE( "Single-channel interleaved frame view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5};
  auto buf = buffer(data, 1, buffer_order::interleaved);

  auto frames = buf.frames();
  REQUIRE(!frames.empty());
  REQUIRE(frames.size() == 5);
}

TEST_CASE( "Single-channel deinterleaved channel view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5};
  auto buf = buffer(data, 1, buffer_order::deinterleaved);

  auto channels = buf.channels();
  REQUIRE(!channels.empty());
  REQUIRE(channels.size() == 1);
}

TEST_CASE( "Single-channel deinterleaved frame view", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5};
  auto buf = buffer(data, 1, buffer_order::deinterleaved);

  auto frames = buf.frames();
  REQUIRE(!frames.empty());
  REQUIRE(frames.size() == 5);
}

TEST_CASE( "Multi-channel interleaved channel view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);

  auto channels = buf.channels();
  REQUIRE(!channels.empty());
  REQUIRE(channels.size() == 3);
}

TEST_CASE( "Multi-channel interleaved frame view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);

  auto frames = buf.frames();
  REQUIRE(!frames.empty());
  REQUIRE(frames.size() == 2);
}

TEST_CASE( "Multi-channel deinterleaved channel view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);

  auto channels = buf.channels();
  REQUIRE(!channels.empty());
  REQUIRE(channels.size() == 3);
}

TEST_CASE( "Multi-channel deinterleaved frame view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);

  auto frames = buf.frames();
  REQUIRE(!frames.empty());
  REQUIRE(frames.size() == 2);
}

TEST_CASE( "Interleaved channel view elements", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);

  auto channels = buf.channels();
  REQUIRE(channels[0].size() == 2);
  REQUIRE(channels[0][0] == 1);
  REQUIRE(channels[0][1] == 4);

  REQUIRE(channels[1].size() == 2);
  REQUIRE(channels[1][0] == 2);
  REQUIRE(channels[1][1] == 5);

  REQUIRE(channels[2].size() == 2);
  REQUIRE(channels[2][0] == 3);
  REQUIRE(channels[2][1] == 6);
}

TEST_CASE( "Interleaved frame view elements", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);

  auto frames = buf.frames();
  REQUIRE(frames[0].size() == 3);
  REQUIRE(frames[0][0] == 1);
  REQUIRE(frames[0][1] == 2);
  REQUIRE(frames[0][2] == 3);

  REQUIRE(frames[1].size() == 3);
  REQUIRE(frames[1][0] == 4);
  REQUIRE(frames[1][1] == 5);
  REQUIRE(frames[1][2] == 6);
}

TEST_CASE( "Deinterleaved channel view elements", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);

  auto channels = buf.channels();
  REQUIRE(channels[0].size() == 2);
  REQUIRE(channels[0][0] == 1);
  REQUIRE(channels[0][1] == 2);

  REQUIRE(channels[1].size() == 2);
  REQUIRE(channels[1][0] == 3);
  REQUIRE(channels[1][1] == 4);

  REQUIRE(channels[2].size() == 2);
  REQUIRE(channels[2][0] == 5);
  REQUIRE(channels[2][1] == 6);
}

TEST_CASE( "Deinterleaved frame view elements", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);

  auto frames = buf.frames();
  REQUIRE(frames[0].size() == 3);
  REQUIRE(frames[0][0] == 1);
  REQUIRE(frames[0][1] == 3);
  REQUIRE(frames[0][2] == 5);

  REQUIRE(frames[1].size() == 3);
  REQUIRE(frames[1][0] == 2);
  REQUIRE(frames[1][1] == 4);
  REQUIRE(frames[1][2] == 6);
}

