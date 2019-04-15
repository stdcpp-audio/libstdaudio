// libstdaudio
// Copyright (c) 2019 - Jean-Michaël Celerier
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once
#include <__audio_base.h>
#include <__audio_device.h>
#include <__audio_buffer.h>
#include <__dynamic_library.h>
#include <__audio_exception.h>
#include <forward_list>
#include <functional>
#include <cassert>
#include <portaudio.h>

// multi-API problem : can't e.g. initialize jack from itself, and also from portaudio

_LIBSTDAUDIO_NAMESPACE_BEGIN

class __portaudio_lib
{
public:
    decltype(&::Pa_Initialize) initialize{}, actual_init{};
    decltype(&::Pa_Terminate) terminate{}, actual_term{};
    decltype(&::Pa_OpenStream) open_stream{};
    decltype(&::Pa_StartStream) start_stream{};
    decltype(&::Pa_CloseStream) close_stream{};
    decltype(&::Pa_StopStream) stop_stream{};
    decltype(&::Pa_IsStreamActive) is_stream_active{};
    decltype(&::Pa_GetDeviceCount) get_device_count{};
    decltype(&::Pa_GetDeviceInfo) get_device_info{};
    decltype(&::Pa_GetDefaultInputDevice) get_default_input{};
    decltype(&::Pa_GetDefaultOutputDevice) get_default_output{};
    decltype(&::Pa_GetErrorText) get_error_text{};

    static const __portaudio_lib& instance() {
        static const __portaudio_lib self;
        return self;
    }


    struct scoped_init_context {
        const __portaudio_lib& library;
        scoped_init_context(const __portaudio_lib& self)
            : library{self}
        {
            auto err = library.initialize();
            if(err != paNoError)
                throw std::runtime_error("Cannot initialize portaudio device");
        }

        ~scoped_init_context()
        {
            library.terminate();
        }
    };

    auto scoped_init() const
    {
        return scoped_init_context{*this};
    }
private:
#if __STDAUDIO_HAS_DYLIB
    __dynamic_library library;

    __portaudio_lib()
  #if defined(_WIN32)
          :library("portaudio.dll")
  #elif defined(__APPLE__)
          :library("portaudio.dylib")
  #else
          :library("libportaudio.so")
  #endif
    {
        actual_init = library.symbol<decltype(&::Pa_Initialize)>("Pa_Initialize");
        actual_term = library.symbol<decltype(&::Pa_Terminate)>("Pa_Terminate");
        initialize = [] {
            return instance().actual_init();
        };
        terminate = [] {
            return instance().actual_term();
        };
        open_stream = library.symbol<decltype(&::Pa_OpenStream)>("Pa_OpenStream");
        start_stream = library.symbol<decltype(&::Pa_StartStream)>("Pa_StartStream");
        close_stream = library.symbol<decltype(&::Pa_CloseStream)>("Pa_CloseStream");
        stop_stream = library.symbol<decltype(&::Pa_StopStream)>("Pa_StopStream");
        is_stream_active = library.symbol<decltype(&::Pa_IsStreamActive)>("Pa_IsStreamActive");
        get_device_count = library.symbol<decltype(&::Pa_GetDeviceCount)>("Pa_GetDeviceCount");
        get_device_info = library.symbol<decltype(&::Pa_GetDeviceInfo)>("Pa_GetDeviceInfo");
        get_default_input = library.symbol<decltype(&::Pa_GetDefaultInputDevice)>("Pa_GetDefaultInputDevice");
        get_default_output = library.symbol<decltype(&::Pa_GetDefaultOutputDevice)>("Pa_GetDefaultOutputDevice");
        get_error_text = library.symbol<decltype(&::Pa_GetErrorText)>("Pa_GetErrorText");
        assert(initialize);
        assert(terminate);
        assert(open_stream);
        assert(start_stream);
        assert(close_stream);
        assert(stop_stream);
        assert(is_stream_active);
        assert(get_device_count);
        assert(get_device_info);
        assert(get_default_input);
        assert(get_default_output);
        assert(get_error_text);
    }
#else
    __portaudio_lib()
    {
        initialize = &::Pa_Initialize;
        terminate = &::__portaudio_lib::instance().terminate;
        open_stream = &::Pa_OpenStream;
        start_stream = &::Pa_StartStream;
        close_stream = &::Pa_CloseStream;
        stop_stream = &::Pa_StopStream;
        is_stream_active = &::Pa_IsStreamActive;
        get_device_count = &::Pa_GetDeviceCount;
        get_device_info = &::Pa_GetDeviceInfo;
        get_default_input = &::Pa_GetDefaultInputDevice;
        get_default_output = &::Pa_GetDefaultOutputDevice;
        get_error_text = &::Pa_GetErrorText;
    }
#endif
};

using __portaudio_device = audio_basic_device<__portaudio_driver_t>;
// How to handle stuff like MME or pulseaudio where you can mix and match sound
// cards ?
template <> class audio_basic_device<__portaudio_driver_t> {
  const PaDeviceInfo &m_info;
  const int m_index;

public:
  audio_basic_device(const PaDeviceInfo &info, PaDeviceIndex device)
      : m_info{info}, m_index{device} {
    if (__portaudio_lib::instance().initialize() != paNoError) {
      throw std::runtime_error("Cannot initialize portaudio device");
    }

    m_in.device = m_index;
    m_in.channelCount = info.maxInputChannels;
    m_in.sampleFormat = paFloat32; // Let's make that configurable later.
    m_in.suggestedLatency = 1024. / info.defaultSampleRate;
    m_in.hostApiSpecificStreamInfo = nullptr;

    m_out.device = m_index;
    m_out.channelCount = info.maxOutputChannels;
    m_out.sampleFormat = paFloat32; // Let's make that configurable later.
    m_out.suggestedLatency = 1024. / info.defaultSampleRate;
    m_out.hostApiSpecificStreamInfo = nullptr;

    m_sampleRate = m_info.defaultSampleRate;
    m_frames = std::max(m_info.defaultHighInputLatency,
                        m_info.defaultHighOutputLatency) *
               m_sampleRate;
  }

  audio_basic_device(const audio_basic_device &other) = delete;
  audio_basic_device(audio_basic_device &&other)
      : audio_basic_device(other.m_info, other.m_index) {}

  audio_basic_device &operator=(const audio_basic_device &) = delete;
  audio_basic_device &operator=(audio_basic_device &&) = delete;
  ~audio_basic_device() { __portaudio_lib::instance().terminate(); }

  using sample_rate_t = double;

  sample_rate_t get_sample_rate() const noexcept { return m_sampleRate; }

  bool set_sample_rate(sample_rate_t new_sample_rate) {
    const bool was_running = is_running();
    if (was_running)
      stop();
    m_sampleRate = new_sample_rate;

    if (was_running)
      start();
    return true;
  }

  bool start() {
    if (is_running())
      return false;

    m_stream = nullptr;

    PaStreamParameters *actualInput{};
    if (m_in.device != paNoDevice && m_in.channelCount > 0)
      actualInput = &m_in;

    PaStreamParameters *actualOutput{};
    if (m_out.device != paNoDevice && m_out.channelCount > 0)
      actualOutput = &m_out;

    // Another thing with portaudio is that you don't know the
    // actual buffer size until it's running - and even then it's only an
    // estimation
    // - on some platforms it can vary per-tick
    auto &pa = __portaudio_lib::instance();
    auto err = pa.open_stream(&m_stream, actualInput, actualOutput,
                              m_sampleRate, m_frames, paNoFlag, callback, this);
    if (err != paNoError) {
      //  std::cerr << pa.get_error_text(err) << "\n";
      return false;
    }
    err = pa.start_stream(m_stream);
    if (err != paNoError) {
      pa.close_stream(m_stream);
      m_stream = nullptr;
      //  std::cerr << pa.get_error_text(err) << "\n";
      return false;
    }

    return true;
  }

  bool stop() {
    if (!is_running()) {
      return false;
    }
    auto &pa = __portaudio_lib::instance();
    auto err = pa.stop_stream(m_stream);
    if (err != paNoError)
      return false;

    err = pa.close_stream(m_stream);
    if (err != paNoError)
      return false;

    m_stream = nullptr;
    return true;
  }

  using buffer_size_t = uint32_t;

  buffer_size_t get_buffer_size_frames() const noexcept { return m_frames; }

  string_view name() const noexcept { return m_info.name; }

  using device_id_t = int;
  device_id_t device_id() const noexcept { return -1; }

  bool is_input() const noexcept { return m_in.channelCount > 0; }

  bool is_output() const noexcept { return m_out.channelCount > 0; }

  int get_num_input_channels() const noexcept { return m_in.channelCount; }

  int get_num_output_channels() const noexcept { return m_out.channelCount; }

  template <typename T> void connect(T t) {
    // in my experience, it is sometimes necessary to swap callbacks at
    // run-time. that's doable with some atomic operations.
    if (is_running())
      throw audio_device_exception("cannot connect to running audio_device");

    m_callback = move(t);
  }

  constexpr bool can_connect() const noexcept { return true; }

  constexpr bool can_process() const noexcept { return false; }

  bool is_running() const noexcept {
    auto &pa = __portaudio_lib::instance();
    return m_stream && pa.is_stream_active(m_stream);
  }

  // Problem for this : depending on the API, it's a pair <sample rate, buffer
  // size> which is supported.
  span<const sample_rate_t> get_supported_sample_rates() const noexcept {
    static constexpr std::array<sample_rate_t, 2> srs{44100., 48000.};
    return {srs.data(), srs.size()};
  }

  span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept {
    static constexpr std::array<buffer_size_t, 7> bs{
        paFramesPerBufferUnspecified, 32, 64, 128, 256, 512, 1024};
    return {bs.data(), bs.size()};
  }

  bool set_buffer_size_frames(buffer_size_t new_buffer_size) {
    const bool was_running = is_running();
    if (was_running)
      stop();

    m_frames = new_buffer_size;

    if (was_running)
      start();
    return true;
  }

private:
  double m_sampleRate{};
  buffer_size_t m_frames{};
  PaStreamParameters m_in{};
  PaStreamParameters m_out{};
  PaStream *m_stream{};

  using __portaudio_callback_t =
      function<void(__portaudio_device &, audio_device_buffers &)>;
  __portaudio_callback_t m_callback;

  static int callback(const void *input, void *output, unsigned long nframes,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData) {
    auto &self = *static_cast<audio_basic_device *>(userData);
    if (self.m_callback) {
      audio_device_buffers bufs;
      // Right now most efficient way I know of doing conversions is using
      // FFMPEG's libsamplerate. How would that fit in ?
      using buffer_t =
          audio_basic_buffer<float,
                             std::experimental::audio_buffer_order_interleaved>;
      if (input) {
        // needs a const_cast here :x
        buffer_t inb((float *)input, nframes * self.get_num_input_channels(),
                     self.get_num_input_channels());
        bufs.__input_buffer = inb;
      }

      if (output) {
        buffer_t outb((float *)output, nframes * self.get_num_output_channels(),
                      self.get_num_output_channels());
        bufs.__output_buffer = outb;
      }

      self.m_callback(self, bufs);
    }
    return paNoError;
  }
};

template <>
class audio_basic_device_list<__portaudio_driver_t>
    : public forward_list<__portaudio_device> {
  const __portaudio_lib &library;
  __portaudio_lib::scoped_init_context context;

public:
  audio_basic_device_list(bool init_inputs, bool init_outputs)
      : library{__portaudio_lib::instance()}, context{library} {
    for (PaDeviceIndex i = 0, n = library.get_device_count(); i < n; i++) {
      try {
        auto dev = library.get_device_info(i);
        if (!dev)
          continue;
        if ((init_inputs && dev->maxInputChannels > 0) ||
            (init_outputs && dev->maxOutputChannels > 0))
          this->emplace_front(*dev, i);
      } catch (...) {
      }
    }
  }
};

template <>
auto get_default_audio_input_device<__portaudio_driver_t>()
    -> optional<audio_basic_device<__portaudio_driver_t>> {
  auto &pa = __portaudio_lib::instance();
  auto context = pa.scoped_init();
  auto idx = pa.get_default_input();
  auto dev = pa.get_device_info(idx);
  if (dev) {
    return audio_basic_device<__portaudio_driver_t>{*dev, idx};
  }
  return {};
}

template <>
auto get_default_audio_output_device<__portaudio_driver_t>()
    -> optional<audio_basic_device<__portaudio_driver_t>> {
  auto &pa = __portaudio_lib::instance();
  auto context = pa.scoped_init();
  auto idx = pa.get_default_output();
  auto dev = pa.get_device_info(idx);
  if (dev) {
    return audio_basic_device<__portaudio_driver_t>{*dev, idx};
  }
  return {};
}

template <>
auto get_audio_input_device_list<__portaudio_driver_t>()
    -> audio_basic_device_list<__portaudio_driver_t> {
  return audio_basic_device_list<__portaudio_driver_t>{true, false};
}

template <>
auto get_audio_output_device_list<__portaudio_driver_t>()
    -> audio_basic_device_list<__portaudio_driver_t> {
  return audio_basic_device_list<__portaudio_driver_t>{false, true};
}

_LIBSTDAUDIO_NAMESPACE_END
