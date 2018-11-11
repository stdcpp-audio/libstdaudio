// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "audio.h"
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Null device name", "[device]") {
  audio::device d;
  REQUIRE(d.name().empty());
}

TEST_CASE( "Null device has no input/output", "[device]") {
  audio::device d;
  REQUIRE(!d.is_input());
  REQUIRE(!d.is_output());
}

TEST_CASE( "Null device calling callback", "[device]") {
  int cb_called = 0;

  audio::device::callback valid_cb = [&](audio::device&, audio::buffer_list& bl){
    cb_called++;
  };

  audio::device d;
  d.process();
  d.process();
  REQUIRE(cb_called == 0);

  d.connect(valid_cb);

  d.process();
  d.process();
  REQUIRE(cb_called == 2);

  audio::device::callback invalid_cb;
  d.connect(invalid_cb);

  d.process();
  d.process();
  REQUIRE(cb_called == 2);
}

TEST_CASE( "Null device passing empty buffer list", "[device]") {
  int num_input_buffers = -1;
  int num_output_buffers = -1;

  audio::device::callback cb = [&](audio::device&, audio::buffer_list& bl){
    num_input_buffers = bl.num_input_buffers();
    num_output_buffers = bl.num_output_buffers();
  };

  audio::device d;
  d.connect(cb);
  d.process();
  REQUIRE(num_input_buffers == 0);
  REQUIRE(num_output_buffers == 0);
}