// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <iostream>
#include <audio>

using namespace std::experimental;

void print_device_info(audio::device& d) {
  std::cout << "- \"" << d.name() << "\"\n";
};

void print_device_list(audio::device_list& list) {
  for (auto& item : list) {
    print_device_info(item);
  }
}

int main() {
  std::cout << "Input devices:\n==============\n";
  print_device_list(audio::get_input_device_list());

  std::cout << "\nOutput devices:\n===============\n";
  print_device_list(audio::get_output_device_list());
}