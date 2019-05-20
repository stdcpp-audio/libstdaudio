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
  using namespace std::experimental;

  auto device = get_default_audio_output_device();
  if (!device)
    return 1;

  const float frequency_hz = 440.0f;
  const float delta = 2.0f * frequency_hz * float(M_PI / device->get_sample_rate());
  float phase = 0;

  device->connect([=](audio_device&, audio_device_io<float>& io) mutable noexcept {
    if (!io.output_buffer.has_value())
      return;

    auto& out = *io.output_buffer;

    for (int frame = 0; frame < out.size_frames(); ++frame) {
      const float next_sample = std::sin(phase);
      phase = std::fmod(phase + delta, 2.0f * float(M_PI));

      for (int channel = 0; channel < out.size_channels(); ++channel)
        out(frame, channel) = 0.2f * next_sample;
    }
  });

  device->start();
  std::this_thread::sleep_for(std::chrono::seconds(5));
}
