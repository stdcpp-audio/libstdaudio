// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "audio.h"
#include "catch/catch.hpp"

using namespace std::experimental;

float data[] = {0, 1, 0, -1};

TEST_CASE( "Buffer default constructor", "[buffer]") {
  auto buf = audio::buffer();
  REQUIRE(buf.raw().data() == nullptr);
  REQUIRE(buf.raw().size() == 0);

  // TODO: is this the correct default? Or should we have buffer_ordering::none?
  // On the other hand, none would create a new class invariant and a new constructor
  // precondition (ordering shall be none if no data, and non-none if there is data)
  REQUIRE(buf.get_ordering() == audio::buffer_ordering::interleaved);
}

// TODO: Test for what should happen if the number of channels and total size don't add up to an integer frame count

TEST_CASE( "Buffer raw data access", "[buffer]") {
  auto buf = audio::buffer(data, 1, audio::buffer_ordering::interleaved);
  REQUIRE(buf.raw().data() == data);
  REQUIRE(buf.raw().size() == sizeof(data) / sizeof(data[0]));
  REQUIRE(buf.get_ordering() == audio::buffer_ordering::interleaved);

  buf = audio::buffer(data, 1, audio::buffer_ordering::deinterleaved);
  REQUIRE(buf.get_ordering() == audio::buffer_ordering::deinterleaved);
}

TEST_CASE( "Buffer channels size", "[buffer]") {
  auto buf = audio::buffer();
  REQUIRE(buf.channels().size() == 0);

  buf = audio::buffer(data, 1, audio::buffer_ordering::interleaved);
  REQUIRE(buf.channels().size() == 1);

  buf = audio::buffer(data, 2, audio::buffer_ordering::interleaved);
  REQUIRE(buf.channels().size() == 2);
}

TEST_CASE( "Buffer channels loop", "[buffer]") {
  auto buf = audio::buffer();
  for (auto& ch : buf.channels())
    FAIL();
}

TEST_CASE( "Buffer channels begin and end", "[buffer]") {
  auto buf = audio::buffer();

  auto ch_begin = buf.channels().begin();
  auto ch_end = buf.channels().end();
  REQUIRE(ch_begin == ch_end);
}