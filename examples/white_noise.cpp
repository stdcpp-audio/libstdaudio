// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <random>
#include <audio.h>

int main() {
  using namespace std::experimental;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> white_noise(-1.0f, 1.0f);

  auto d = audio::get_output_device();
  d.connect([&](audio::device&, audio::buffer_list& bl){
    for (auto& buffer : bl.output_buffers())
      for (auto& sample : buffer.raw())
        sample = white_noise(gen);
  });

  while(true);
}