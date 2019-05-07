// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <random>
#include <thread>
#include <audio>

// This example app outputs 5 seconds of white noise.

int main() {
  using namespace std::experimental;

  std::random_device rd;
  std::minstd_rand gen(rd());
  std::uniform_real_distribution<float> white_noise(-1.0f, 1.0f);

  auto device = get_default_audio_output_device();
  if (!device)
    return 1;

  device->connect([&](audio_device&, audio_device_io<float>& io){
    if (!io.output_buffer.has_value())
      return;

    auto& out = *io.output_buffer;
    for (auto& sample : out.samples())
      sample = white_noise(gen);
  });

  device->start();
  std::this_thread::sleep_for(std::chrono::seconds(2));
  device->stop();
}
