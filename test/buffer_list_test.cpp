// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental::audio;

TEST_CASE( "Construct empty buffer list", "[buffer_list]") {
  auto bl = buffer_list();

  REQUIRE(bl.input_buffers().empty());
  REQUIRE(bl.output_buffers().empty());
}

TEST_CASE( "Construct non-empty buffer list", "[buffer_list]") {
  auto bl = buffer_list(2, 3);

  REQUIRE(bl.input_buffers().size() == 2);
  REQUIRE(bl.output_buffers().size() == 3);
}

TEST_CASE( "Resize buffer list", "[buffer_list]") {
  auto bl = buffer_list(2, 3);
  bl.resize(4, 5);

  REQUIRE(bl.input_buffers().size() == 4);
  REQUIRE(bl.output_buffers().size() == 5);
}