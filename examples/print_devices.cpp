// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <iostream>
#include <audio>

using namespace std::experimental;

void print_device_info(audio::device& d) {
  std::cout << "- \"" << d.name() << "\", ";
  std::cout << (d.get_native_order() == audio::buffer_order::deinterleaved ? "de" : "") << "interleaved, ";
  std::cout << "sample rate = " << d.get_sample_rate() << " Hz, ";
  std::cout << "buffer size = " << d.get_buffer_size_bytes() << " bytes\n";
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