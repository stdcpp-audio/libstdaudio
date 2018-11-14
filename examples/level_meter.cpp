// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <iostream>
#include <atomic>
#include <thread>
#include <audio.h>

int main() {
  using namespace std::experimental;
  std::atomic<float> max_volume = 0;

  auto d = audio::get_input_device();
  d.connect([&](audio::device&, audio::buffer_list& bl){
    float new_max_volume = 0;

    for (auto& buffer : bl.input_buffers()) {
      for (auto& sample : buffer.raw()) {
        if (sample > new_max_volume)
          new_max_volume = sample;
      }
    }

    max_volume.store(new_max_volume);
  });

  while(true) {
    max_volume.store(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << max_volume << std::endl;
  }
}