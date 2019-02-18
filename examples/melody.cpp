// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <cmath>
#include <array>
#include <thread>
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

struct synth {
  float get_next_sample() {
    assert (_sample_rate > 0);

    _ms_counter += 1000. / _sample_rate;
    if (_ms_counter >= _note_duration_ms) {
      _ms_counter = 0;
      _current_note_index++;
      update();
    }

    if (_current_note_index >= notes.size()) {
      stop.store(true);
      return 0;
    }

    auto next_sample = std::copysign(1.0f, std::sin(_phase));
    _phase = std::fmod(_phase + _delta, 2.0f * float(M_PI));
    return next_sample;
  }

  void set_sample_rate(float sample_rate) {
    _sample_rate = sample_rate;
    update();
  }

private:
  void update() noexcept {
    float frequency_hz = note_to_frequency_hz(notes[_current_note_index]);
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
  using namespace std::experimental::audio;

  auto d = get_default_output_device();
  auto s = synth{};
  s.set_sample_rate(float(d.get_sample_rate()));

  d.connect([=](device&, buffer_list& bl) mutable {
    for (auto& buffer : bl.output_buffers()) {
      for (auto& frame : buffer.frames()) {
        auto next_sample = s.get_next_sample();
        for (auto& sample : frame)
          sample = next_sample;
      }
    }
  });

  d.start();
  while (!stop.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  d.stop();
}