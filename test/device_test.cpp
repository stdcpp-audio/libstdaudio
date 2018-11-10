// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include "audio.h"
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Null device", "[device]") {
  int cb_called = 0;
  audio::device::callback valid_cb = [&](audio::device&, audio::buffer_list&){
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

