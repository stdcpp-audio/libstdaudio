// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <cmath>
#include <thread>
#include <audio>

// This example app plays a sine wave of a given frequency for 5 seconds.

int main() {
  using namespace std::experimental;

  auto device = get_default_audio_output_device();
  if (!device)
    return 1;

  float frequency_hz = 440.0f;
  float delta = 2.0f * frequency_hz * float(M_PI / device->get_sample_rate());
  float phase = 0;

  device->connect([=](audio_device& device, audio_device_buffers& buffers) mutable {
    auto buffer = *buffers.output_buffer();
    for (int frame = 0; frame < buffer.size_frames(); ++frame) {
      float next_sample = std::sin(phase);
      phase = std::fmod(phase + delta, 2.0f * M_PI);
      for (int channel = 0; channel < buffer.size_channels(); ++channel) {
        buffer(frame, channel) = next_sample;
      }
    }
  });

  device->start();
  std::this_thread::sleep_for(std::chrono::seconds(2));
  device->stop();
}