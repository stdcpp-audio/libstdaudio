// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "audio.h"
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Null device", "[device]") {
  int cb_called = 0;
  int num_input_buffers = -1;
  int num_output_buffers = -1;

  audio::device::callback valid_cb = [&](audio::device&, audio::buffer_list& bl){
    cb_called++;
    num_input_buffers = bl.num_input_buffers();
    num_output_buffers = bl.num_output_buffers();
  };

  audio::device d;
  d.process();
  d.process();
  REQUIRE(cb_called == 0);

  d.connect(valid_cb);

  d.process();
  d.process();
  REQUIRE(cb_called == 2);
  REQUIRE(num_input_buffers == 0);
  REQUIRE(num_output_buffers == 0);

  audio::device::callback invalid_cb;
  d.connect(invalid_cb);

  d.process();
  d.process();
  REQUIRE(cb_called == 2);
}

