// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <iostream>
#include <audio.h>

using namespace std::experimental;

void print_device_info(audio::device& d, audio::buffer_list& bl) {
  std::cout << "- \"" << d.name() << "\"\n";
  std::cout << "  -> " << bl.num_input_buffers() << " input buffers\n";
  std::cout << "  -> " << bl.num_output_buffers() << " output buffers\n";
};

void print_device_list(audio::device_list& list) {
  for (auto& item : list) {
    item.connect(&print_device_info);
    item.process();
  }
}

int main() {
  std::cout << "Input devices:\n==============\n";
  print_device_list(audio::get_input_device_list());

  std::cout << "\nOutput devices:\n===============\n";
  print_device_list(audio::get_output_device_list());
}