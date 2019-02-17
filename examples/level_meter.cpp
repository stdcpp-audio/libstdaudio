// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <atomic>
#include <thread>
#include <audio>

int main() {
  using namespace std::experimental::audio;
  std::atomic<float> max_volume = 0;

  auto d = get_default_input_device();
  d.connect([&](device&, buffer_list& bl){
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