// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <iostream>
#include <audio.h>

using namespace std::experimental;

void print_device_list(audio::device_list& list) {
  int i = 0;
  for (auto& item : list) {
    std::cout << "- device " << i++ << ": \"" << item.name() << "\"\n";
  }
}

int main() {
  std::cout << "Input devices:\n==============\n";
  print_device_list(audio::get_input_device_list());

  std::cout << "\nOutput devices:\n===============\n";
  print_device_list(audio::get_output_device_list());
}