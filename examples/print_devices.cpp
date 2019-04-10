// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <audio>

using namespace std::experimental;

void print_device_info(const audio_device& d) {
  std::cout << "- \"" << d.name() << "\", ";
  std::cout << "sample rate = " << d.get_sample_rate() << " Hz, ";
  std::cout << "buffer size = " << d.get_buffer_size_frames() << " frames, ";
  std::cout << (d.is_input() ? d.get_num_input_channels() : d.get_num_output_channels()) << " channels\n";
};

void print_device_list(const audio_device_list& list) {
  for (auto& item : list) {
    print_device_info(item);
  }
}

int main() {
  std::cout << "Input devices:\n==============\n";
  print_device_list(get_audio_input_device_list());

  std::cout << "\nOutput devices:\n===============\n";
  print_device_list(get_audio_output_device_list());
}