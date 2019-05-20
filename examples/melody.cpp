// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif

#include <cmath>
#include <array>
#include <thread>
#include <atomic>
#include <audio>

// This example app plays a short melody using a simple square wave synthesiser.

constexpr std::array<int, 22> notes = {
  88, 86, 78, 78, 80, 80,
  85, 83, 74, 74, 76, 76,
  83, 81, 73, 73, 76, 76,
  81, 81, 81, 81
};

constexpr float bpm = 260.0;

float note_to_frequency_hz(int note) {
  constexpr float pitch_standard_hz = 440.0f;
  return pitch_standard_hz * std::pow(2.0f, float(note - 69) / 12.0f);
}

std::atomic<bool> stop = false;

struct synthesiser {
  float get_next_sample() {
    assert (_sample_rate > 0);

    _ms_counter += 1000.0f / _sample_rate;
    if (_ms_counter >= _note_duration_ms) {
      _ms_counter = 0;
      if (++_current_note_index < notes.size()) {
        update();
      }
      else {
        stop.store(true);
        return 0;
      }
    }

    const auto next_sample = std::copysign(0.1f, std::sin(_phase));
    _phase = std::fmod(_phase + _delta, 2.0f * float(M_PI));
    return next_sample;
  }

  void set_sample_rate(float sample_rate) {
    _sample_rate = sample_rate;
    update();
  }

private:
  void update() noexcept {
    const float frequency_hz = note_to_frequency_hz(notes.at(_current_note_index));
    _delta = 2.0f * frequency_hz * static_cast<float>(M_PI / _sample_rate);
  }

  float _sample_rate = 0;
  float _delta = 0;
  float _phase = 0;
  float _ms_counter = 0;
  float _note_duration_ms = 60'000.0f / bpm;
  int _current_note_index = 0;
};


int main() {
  using namespace std::experimental;

  auto device = get_default_audio_output_device();
  if (!device)
    return 1;

  auto synth = synthesiser();
  synth.set_sample_rate(float(device->get_sample_rate()));

  device->connect([=](audio_device&, audio_device_io<float>& io) mutable noexcept {
    if (!io.output_buffer.has_value())
      return;

    auto& out = *io.output_buffer;

    for (int frame = 0; frame < out.size_frames(); ++frame) {
      auto next_sample = synth.get_next_sample();

      for (int channel = 0; channel < out.size_channels(); ++channel)
        out(frame, channel) = next_sample;
    }
  });

  device->start();
  while (!stop.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}
