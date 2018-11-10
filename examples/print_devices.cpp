// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <iostream>
#include <audio.h>

int main() {
  using namespace std::experimental;

  std::cout << "Input devices:\n"
               "==============\n";

  auto& input_devices = audio::get_input_device_list();
  for (auto& input_device : input_devices) {
    // TODO
  }

  std::cout << "\nOutput devices:\n"
               "===============\n";

  auto& output_devices = audio::get_output_device_list();
  for (auto& output_device : output_devices) {
    // TODO
  }
}