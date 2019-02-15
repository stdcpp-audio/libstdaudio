// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Construct empty buffer list", "[buffer_list]") {
  auto bl = audio::buffer_list();

  REQUIRE(bl.input_buffers().size() == 0);
  REQUIRE(bl.output_buffers().size() == 0);
}

TEST_CASE( "Construct non-empty buffer list", "[buffer_list]") {
  auto bl = audio::buffer_list(2, 3);

  REQUIRE(bl.input_buffers().size() == 2);
  REQUIRE(bl.output_buffers().size() == 3);
}

TEST_CASE( "Resize buffer list", "[buffer_list]") {
  auto bl = audio::buffer_list(2, 3);
  bl.resize(4, 5);

  REQUIRE(bl.input_buffers().size() == 4);
  REQUIRE(bl.output_buffers().size() == 5);
}