// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <random>
#include <thread>
#include <audio>

// This example app outputs 5 seconds of white noise.

int main() {
  using namespace std::experimental::audio;

  std::random_device rd;
  std::minstd_rand gen(rd());
  std::uniform_real_distribution<float> white_noise(-1.0f, 1.0f);

  auto d = get_default_output_device();
  d.connect([&](device&, buffer_list& bl){
    for (auto& buffer : bl.output_buffers())
      for (auto& sample : buffer.raw())
        sample = white_noise(gen);
  });

  d.start();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  d.stop();
}