// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <audio.h>

int main() {
  using namespace std::experimental;

  auto d = audio::get_output_device();
  d.connect([](audio::device&, audio::buffer_list&){
    // TODO: implement white noise generator here
  });

  while(true);
}