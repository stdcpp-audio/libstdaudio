// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iomanip>
#include <atomic>
#include <cmath>
#include <thread>
#include <audio>

// This example app prints the current input level in regular intervals.

float gain_to_db(float gain) noexcept {
  return gain > 0 ? std::log10(gain) * 20.0f : -std::numeric_limits<float>::infinity();
}

int main() {
  using namespace std::experimental;
  std::atomic<float> max_abs_value = 0;

  auto device = get_default_audio_input_device();
  if (!device)
    return 1;

  device->connect([&](audio_device&, audio_device_buffers& buffers){
    auto buffer = *buffers.input_buffer();
    float* samples = buffer.data();
    for (int i = 0; i < buffer.size_samples(); ++i) {
      float abs_value = std::abs(samples[i]);
      if (abs_value > max_abs_value)
        max_abs_value.store(abs_value);
    }
  });

  device->start();
  while(true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    std::cout << gain_to_db(max_abs_value.exchange(0)) << " dB\n";
  }

  device->stop();
}