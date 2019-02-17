// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental::audio;

TEST_CASE( "Buffer default constructor", "[buffer]") {
  auto buf = buffer();
  REQUIRE(buf.raw().data() == nullptr);
  REQUIRE(buf.raw().size() == 0);

  // TODO: is this the correct default? Or should we have buffer_order::none?
  // On the other hand, none would create a new class invariant and a new constructor
  // precondition (ordering shall be none if no data, and non-none if there is data)
  REQUIRE(buf.get_order() == buffer_order::interleaved);
}

TEST_CASE( "Buffer empty()", "[buffer]") {
  auto buf = buffer();
  REQUIRE(buf.empty());

  float data[] = {0, 1, 0, -1};
  buf = buffer(data, 1, buffer_order::interleaved);
  REQUIRE(!buf.empty());
}

TEST_CASE( "Buffer size_bytes()", "[buffer]") {
  auto buf = buffer();
  REQUIRE(buf.size_bytes() == 0);

  float data[] = {0, 1, 0, -1};
  buf = buffer(data, 1, buffer_order::interleaved);
  REQUIRE(buf.size_bytes() == 16);

  buf = buffer(data, 4, buffer_order::interleaved);
  REQUIRE(buf.size_bytes() == 16);
}

TEST_CASE( "Buffer raw data access", "[buffer]") {
  auto buf = buffer();
  REQUIRE(buf.raw().empty());

  float data[] = {0, 1, 0, -1};
  buf = buffer(data, 1, buffer_order::interleaved);

  REQUIRE(!buf.raw().empty());
  REQUIRE(buf.raw().data() == data);
  REQUIRE(buf.raw().size() == sizeof(data) / sizeof(data[0]));
  REQUIRE(buf.get_order() == buffer_order::interleaved);

  buf = buffer(data, 1, buffer_order::deinterleaved);
  REQUIRE(!buf.raw().empty());
  REQUIRE(buf.get_order() == buffer_order::deinterleaved);
}
