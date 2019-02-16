// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <iostream>
#include <audio>

using namespace std::experimental::audio;

void print_device_info(device& d) {
  std::cout << "- \"" << d.name() << "\", ";
  std::cout << (d.get_native_order() == buffer_order::deinterleaved ? "de" : "") << "interleaved, ";
  std::cout << "sample rate = " << d.get_sample_rate() << " Hz, ";
  std::cout << "buffer size = " << d.get_buffer_size_bytes() << " bytes\n";
};

void print_device_list(device_list& list) {
  for (auto& item : list) {
    print_device_info(item);
  }
}

int main() {
  std::cout << "Input devices:\n==============\n";
  print_device_list(get_input_device_list());

  std::cout << "\nOutput devices:\n===============\n";
  print_device_list(get_output_device_list());
}