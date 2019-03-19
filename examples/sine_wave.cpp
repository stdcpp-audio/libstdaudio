// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <thread>
#include <audio>

// This example app plays a sine wave of a given frequency for 5 seconds.

int main() {
  using namespace std::experimental::audio;

  auto d = get_default_output_device();
  float frequency_hz = 440.0f;
  float delta = 2.0f * frequency_hz * float(M_PI / d.get_sample_rate());
  float phase = 0;

  d.connect([=](device&, buffer_list& bl) mutable {
    for (auto& buffer : bl.output_buffers()) {
      for (auto& frame : buffer.frames()) {
        float next_sample = std::sin(phase);
        phase += delta;
        for (auto& sample : frame)
          sample = next_sample;
      }
    }
  });

  d.start();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  d.stop();
}