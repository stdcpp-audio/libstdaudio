// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "audio.h"
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Empty buffer list", "[buffer_list]") {
  audio::buffer_list bl;

  REQUIRE(bl.num_input_buffers() == 0);
  REQUIRE(bl.num_output_buffers() == 0);
}