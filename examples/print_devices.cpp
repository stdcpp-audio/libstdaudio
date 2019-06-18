// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <audio>
#include <thread>

using namespace std::experimental;

bool is_default_device(const audio_device& d) {
  if (d.is_input()) {
    auto default_in = get_default_audio_input_device();
    return default_in.has_value() && d.device_id() == default_in->device_id();
  }
  else if (d.is_output()) {
    auto default_out = get_default_audio_output_device();
    return default_out.has_value() && d.device_id() == default_out->device_id();
  }

  return false;
}

void print_device_info(const audio_device& d) {
  std::cout << "- \"" << d.name() << "\", ";
  std::cout << "sample rate = " << d.get_sample_rate() << " Hz, ";
  std::cout << "buffer size = " << d.get_buffer_size_frames() << " frames, ";
  std::cout << (d.is_input() ? d.get_num_input_channels() : d.get_num_output_channels()) << " channels";
  std::cout << (is_default_device(d) ? " [DEFAULT DEVICE]\n" : "\n");
}

void print_device_list(const audio_device_list& list) {
  for (auto& item : list) {
    print_device_info(item);
  }
}

void print_all_devices() {
  std::cout << "Input devices:\n==============\n";
  print_device_list(get_audio_input_device_list());

  std::cout << "\nOutput devices:\n===============\n";
  print_device_list(get_audio_output_device_list());
}

int main() {
  print_all_devices();

  set_audio_device_list_callback(audio_device_list_event::device_list_changed, []{
    std::cout << "\n=== Audio device list changed! ===\n\n";
    print_all_devices();
  });

  set_audio_device_list_callback(audio_device_list_event::default_input_device_changed, []{
    std::cout << "\n=== Default input device changed! ===\n\n";
    print_all_devices();
  });

  set_audio_device_list_callback(audio_device_list_event::default_output_device_changed, []{
    std::cout << "\n=== Default output device changed! ===\n\n";
    print_all_devices();
  });

  std::this_thread::sleep_for(std::chrono::system_clock::duration::max());
}