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
#include <string>
#include <thread>
#include <chrono>
#include <cassert>
#include <iostream>
#include <pulse/pulseaudio.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

class __pulseaudio_lib
{
public:
    decltype(&::pa_threaded_mainloop_new) pa_threaded_mainloop_new{};
    decltype(&::pa_threaded_mainloop_free) pa_threaded_mainloop_free{};
    decltype(&::pa_threaded_mainloop_get_api) pa_threaded_mainloop_get_api{};
    decltype(&::pa_context_new) pa_context_new{};
    decltype(&::pa_context_ref) pa_context_ref{};
    decltype(&::pa_context_unref) pa_context_unref{};
    decltype(&::pa_context_set_state_callback) pa_context_set_state_callback{};
    decltype(&::pa_threaded_mainloop_lock) pa_threaded_mainloop_lock{};
    decltype(&::pa_threaded_mainloop_unlock) pa_threaded_mainloop_unlock{};
    decltype(&::pa_threaded_mainloop_start) pa_threaded_mainloop_start{};
    decltype(&::pa_threaded_mainloop_stop) pa_threaded_mainloop_stop{};
    decltype(&::pa_context_connect) pa_context_connect{};
    decltype(&::pa_context_get_state) pa_context_get_state{};
    decltype(&::pa_threaded_mainloop_wait) pa_threaded_mainloop_wait{};
    decltype(&::pa_channel_map_init_stereo) pa_channel_map_init_stereo{};
    decltype(&::pa_stream_new) pa_stream_new{};
    decltype(&::pa_stream_new_with_proplist) pa_stream_new_with_proplist{};
    decltype(&::pa_stream_ref) pa_stream_ref{};
    decltype(&::pa_stream_unref) pa_stream_unref{};
    decltype(&::pa_stream_get_state) pa_stream_get_state{};
    decltype(&::pa_stream_set_state_callback) pa_stream_set_state_callback{};
    decltype(&::pa_stream_set_write_callback) pa_stream_set_write_callback{};
    decltype(&::pa_stream_set_read_callback) pa_stream_set_read_callback{};
    decltype(&::pa_stream_set_overflow_callback) pa_stream_set_overflow_callback{};
    decltype(&::pa_stream_connect_playback) pa_stream_connect_playback{};
    decltype(&::pa_stream_cork) pa_stream_cork{};
    decltype(&::pa_stream_is_corked) pa_stream_is_corked{};
    decltype(&::pa_threaded_mainloop_signal) pa_threaded_mainloop_signal{};
    decltype(&::pa_stream_begin_write) pa_stream_begin_write{};
    decltype(&::pa_stream_write) pa_stream_write{};
    decltype(&::pa_stream_set_name) pa_stream_set_name{};

    static const __pulseaudio_lib& instance() {
        static const __pulseaudio_lib self;
        return self;
    }

private:
#if __STDAUDIO_HAS_DYLIB
    __dynamic_library library;

    __pulseaudio_lib()
        :library("libpulse.so")
    {
        // in terms of regex:
        // decltype\(&::([a-z_]+)\) [a-z_]+{};
        // \1 = library.symbol<decltype(&::\1)>("\1");

        // we need reflection T_T
        pa_threaded_mainloop_new = library.symbol<decltype(&::pa_threaded_mainloop_new)>("pa_threaded_mainloop_new");
        pa_threaded_mainloop_free = library.symbol<decltype(&::pa_threaded_mainloop_free)>("pa_threaded_mainloop_free");
        pa_threaded_mainloop_get_api = library.symbol<decltype(&::pa_threaded_mainloop_get_api)>("pa_threaded_mainloop_get_api");
        pa_threaded_mainloop_lock = library.symbol<decltype(&::pa_threaded_mainloop_lock)>("pa_threaded_mainloop_lock");
        pa_threaded_mainloop_unlock = library.symbol<decltype(&::pa_threaded_mainloop_lock)>("pa_threaded_mainloop_unlock");
        pa_threaded_mainloop_start = library.symbol<decltype(&::pa_threaded_mainloop_start)>("pa_threaded_mainloop_start");
        pa_threaded_mainloop_stop = library.symbol<decltype(&::pa_threaded_mainloop_stop)>("pa_threaded_mainloop_stop");
        pa_threaded_mainloop_signal = library.symbol<decltype(&::pa_threaded_mainloop_signal)>("pa_threaded_mainloop_signal");
        pa_context_new = library.symbol<decltype(&::pa_context_new)>("pa_context_new");
        pa_context_ref = library.symbol<decltype(&::pa_context_ref)>("pa_context_ref");
        pa_context_unref = library.symbol<decltype(&::pa_context_unref)>("pa_context_unref");
        pa_context_set_state_callback = library.symbol<decltype(&::pa_context_set_state_callback)>("pa_context_set_state_callback");
        pa_context_connect = library.symbol<decltype(&::pa_context_connect)>("pa_context_connect");
        pa_context_get_state = library.symbol<decltype(&::pa_context_get_state)>("pa_context_get_state");
        pa_threaded_mainloop_wait = library.symbol<decltype(&::pa_threaded_mainloop_wait)>("pa_threaded_mainloop_wait");
        pa_channel_map_init_stereo = library.symbol<decltype(&::pa_channel_map_init_stereo)>("pa_channel_map_init_stereo");
        pa_stream_new = library.symbol<decltype(&::pa_stream_new)>("pa_stream_new");
        pa_stream_new_with_proplist = library.symbol<decltype(&::pa_stream_new_with_proplist)>("pa_stream_new_with_proplist");
        pa_stream_ref = library.symbol<decltype(&::pa_stream_ref)>("pa_stream_ref");
        pa_stream_unref = library.symbol<decltype(&::pa_stream_unref)>("pa_stream_unref");
        pa_stream_get_state = library.symbol<decltype(&::pa_stream_get_state)>("pa_stream_get_state");
        pa_stream_set_state_callback = library.symbol<decltype(&::pa_stream_set_state_callback)>("pa_stream_set_state_callback");
        pa_stream_set_write_callback = library.symbol<decltype(&::pa_stream_set_write_callback)>("pa_stream_set_write_callback");
        pa_stream_set_read_callback = library.symbol<decltype(&::pa_stream_set_read_callback)>("pa_stream_set_read_callback");
        pa_stream_set_overflow_callback = library.symbol<decltype(&::pa_stream_set_overflow_callback)>("pa_stream_set_overflow_callback");
        pa_stream_connect_playback = library.symbol<decltype(&::pa_stream_connect_playback)>("pa_stream_connect_playback");
        pa_stream_cork = library.symbol<decltype(&::pa_stream_cork)>("pa_stream_cork");
        pa_stream_is_corked = library.symbol<decltype(&::pa_stream_is_corked)>("pa_stream_is_corked");
        pa_stream_begin_write = library.symbol<decltype(&::pa_stream_begin_write)>("pa_stream_begin_write");
        pa_stream_write = library.symbol<decltype(&::pa_stream_write)>("pa_stream_write");
        pa_stream_set_name = library.symbol<decltype(&::pa_stream_set_name)>("pa_stream_set_name");


        // in terms of regex:
        // decltype\(&::([a-z_]+)\) [a-z_]+{};
        // assert(\1);
        assert(pa_threaded_mainloop_new);
        assert(pa_threaded_mainloop_free);
        assert(pa_threaded_mainloop_get_api);
        assert(pa_threaded_mainloop_lock);
        assert(pa_threaded_mainloop_unlock);
        assert(pa_threaded_mainloop_start);
        assert(pa_threaded_mainloop_stop);
        assert(pa_threaded_mainloop_signal);
        assert(pa_context_new);
        assert(pa_context_ref);
        assert(pa_context_unref);
        assert(pa_context_set_state_callback);
        assert(pa_context_connect);
        assert(pa_context_get_state);
        assert(pa_threaded_mainloop_wait);
        assert(pa_channel_map_init_stereo);
        assert(pa_stream_new);
        assert(pa_stream_new_with_proplist);
        assert(pa_stream_ref);
        assert(pa_stream_unref);
        assert(pa_stream_get_state);
        assert(pa_stream_set_state_callback);
        assert(pa_stream_set_write_callback);
        assert(pa_stream_set_read_callback);
        assert(pa_stream_set_overflow_callback);
        assert(pa_stream_connect_playback);
        assert(pa_stream_cork);
        assert(pa_stream_is_corked);
        assert(pa_stream_begin_write);
        assert(pa_stream_write);
        assert(pa_stream_set_name);
    }
#else
    __pulseaudio_lib()
    {
        // in terms of regex:
        // decltype\(&::([a-z_]+)\) [a-z_]+{};
        // \1 = &::\1;

        pa_threaded_mainloop_new = &::pa_threaded_mainloop_new;
        pa_threaded_mainloop_free = &::pa_threaded_mainloop_free;
        pa_threaded_mainloop_get_api = &::pa_threaded_mainloop_get_api;
        pa_threaded_mainloop_lock = &::pa_threaded_mainloop_lock;
        pa_threaded_mainloop_unlock = &::pa_threaded_mainloop_unlock;
        pa_threaded_mainloop_start = &::pa_threaded_mainloop_start;
        pa_threaded_mainloop_stop = &::pa_threaded_mainloop_stop;
        pa_threaded_mainloop_signal = &::pa_threaded_mainloop_signal;
        pa_context_new = &::pa_context_new;
        pa_context_ref = &::pa_context_ref;
        pa_context_unref = &::pa_context_unref;
        pa_context_connect = &::pa_context_connect;
        pa_context_get_state = &::pa_context_get_state;
        pa_context_set_state_callback = &::pa_context_set_state_callback;
        pa_threaded_mainloop_wait = &::pa_threaded_mainloop_wait;
        pa_channel_map_init_stereo = &::pa_channel_map_init_stereo;
        pa_stream_new = &::pa_stream_new;
        pa_stream_new_with_proplist = &::pa_stream_new_with_proplist;
        pa_stream_ref = &::pa_stream_ref;
        pa_stream_unref = &::pa_stream_unref;
        pa_stream_get_state = &::pa_stream_get_state;
        pa_stream_set_state_callback = &::pa_stream_set_state_callback;
        pa_stream_set_write_callback = &::pa_stream_set_write_callback;
        pa_stream_set_read_callback = &::pa_stream_set_read_callback;
        pa_stream_set_overflow_callback = &::pa_stream_set_overflow_callback;
        pa_stream_connect_playback = &::pa_stream_connect_playback;
        pa_stream_cork = &::pa_stream_cork;
        pa_stream_is_corked = &::pa_stream_is_corked;
        pa_stream_begin_write = &::pa_stream_begin_write;
        pa_stream_write = &::pa_stream_write;
        pa_stream_set_name = &::pa_stream_set_name;
    }
#endif
};

using __pulseaudio_device = audio_basic_device<__pulseaudio_driver_t>;
// How to handle stuff like MME or pulseaudio where you can mix and match sound
// cards ?
template <> class audio_basic_device<__pulseaudio_driver_t> {
public:
  audio_basic_device() noexcept {
    m_sampleRate = 44100;
    m_frames = 512;
    m_name = "My app";
  }

  audio_basic_device(const audio_basic_device &other) = delete;
  audio_basic_device(audio_basic_device &&other) noexcept
      : m_sampleRate(other.m_sampleRate), m_frames(other.m_frames),
        m_mainloop(other.m_mainloop), m_api(other.m_api),
        m_context(other.m_context), m_stream(other.m_stream),
        m_callback(std::move(other.m_callback)) {
    other.m_mainloop = nullptr;
    other.m_api = nullptr;
    other.m_context = nullptr;
    other.m_stream = nullptr;
  }

  audio_basic_device &operator=(const audio_basic_device &) = delete;
  audio_basic_device &operator=(audio_basic_device &&) = delete;
  ~audio_basic_device() { stop(); }

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

    const auto &pa = __pulseaudio_lib::instance();

    m_mainloop = pa.pa_threaded_mainloop_new();
    if (!m_mainloop) {
      // let's switch to Either or Outcome shall we :-)
      throw audio_device_exception("Cannot initialize pulseaudio main loop");
    }

    m_api = pa.pa_threaded_mainloop_get_api(m_mainloop);
    if (!m_api) {
      throw audio_device_exception("Cannot initialize pulseaudio mainloop API");
    }

    // both for pulseaudio, and for JACK, instead of requesting a device,
    // you declare your app as being a device which will be a piece of the
    // audio graph - so there should be an API to set its name in that graph.
    m_context = pa.pa_context_new(m_api, m_name.data());
    if (!m_context) {
      throw audio_device_exception("Cannot initialize pulseaudio context");
    }

    auto context_callback = [](pa_context *, void *mainloop) {
      auto &pa = __pulseaudio_lib::instance();
      pa.pa_threaded_mainloop_signal((pa_threaded_mainloop *)mainloop, 0);
    };
    pa.pa_context_set_state_callback(m_context, context_callback, m_mainloop);

    pa.pa_threaded_mainloop_lock(m_mainloop);

    if (auto err = pa.pa_threaded_mainloop_start(m_mainloop); err != 0) {
      throw audio_device_exception("Cannot start pulseaudio main loop");
    }
    if (auto err = pa.pa_context_connect(m_context, nullptr,
                                         PA_CONTEXT_NOAUTOSPAWN, nullptr);
        err != 0) {
      throw audio_device_exception("Cannot conntext the pulseaudio context");
    }

    // Wait until everything is ready - completely arbitrary timeout, maybe this
    // should be settable
    static const constexpr auto default_timeout = 3s;
    {
      auto t0 = chrono::steady_clock::now();
      bool timeout = false;
      while ((timeout = (chrono::steady_clock::now() - t0 < default_timeout))) {
        switch (pa.pa_context_get_state(m_context)) {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
          pa.pa_threaded_mainloop_wait(m_mainloop);
          continue;

        case PA_CONTEXT_READY:
          break;

        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
        default:
          throw audio_device_exception("Invalid context state");
        }
      }

      if (timeout) {
        throw audio_device_exception("Context creation timeout");
      }
    }


    /// Everything from the beginning of start() to this point should
    /// actually go in a context object which is not part of the device.
    /// (A bit like Pa_Initialize() / Pa_Terminate() for portaudio)

    pa_sample_spec sample_specifications;
    sample_specifications.format = PA_SAMPLE_FLOAT32LE;

    // for pulseaudio, samplerate is an uint
    sample_specifications.rate = m_sampleRate;

    // in pulse and JACK we ask how many channels we want ; in addition to Jack,
    // pulse does intelligent resampling / upmixing / downmixing
    sample_specifications.channels = 2;

    pa_channel_map map;
    pa.pa_channel_map_init_stereo(&map);

    m_stream =
        pa.pa_stream_new(m_context, "My stream", &sample_specifications, &map);
    if (!m_stream) {
      throw audio_device_exception("Cannot initialize pulseaudio stream");
    }

    const auto stream_cb = [](pa_stream *s, void *mainloop) {
      auto &pa = __pulseaudio_lib::instance();
      pa.pa_threaded_mainloop_signal((pa_threaded_mainloop *)mainloop, 0);
    };
    pa.pa_stream_set_state_callback(m_stream, stream_cb, m_mainloop);
    pa.pa_stream_set_write_callback(m_stream, output_callback, this);

    // Don't set values here, instead the server will provide what it judges to
    // be the best.
    pa_buffer_attr buffer_attr;
    buffer_attr.maxlength = m_frames * sizeof(float) * get_num_output_channels();
    buffer_attr.tlength = m_frames * sizeof(float) * get_num_output_channels();
    buffer_attr.prebuf = (uint32_t)-1;
    buffer_attr.minreq = 0;

    const auto stream_flags = static_cast<pa_stream_flags_t>(
        PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_NOT_MONOTONIC |
        PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_ADJUST_LATENCY);

    // Connect stream to the default audio output sink
    if (auto err = pa.pa_stream_connect_playback(
            m_stream, nullptr, &buffer_attr, stream_flags, nullptr, nullptr);
        err != 0) {
      throw audio_device_exception("Cannot start pulseaudio stream");
    }

    {
      auto t0 = chrono::steady_clock::now();
      bool timeout = false;
      while ((timeout = (chrono::steady_clock::now() - t0 < default_timeout))) {
        switch (pa.pa_stream_get_state(m_stream)) {
        case PA_STREAM_CREATING:
          pa.pa_threaded_mainloop_wait(m_mainloop);
          break;
        case PA_STREAM_READY:
          break;
        default:
          throw audio_device_exception("Invalid stream state");
        }
      }

      if (timeout) {
        throw audio_device_exception("Stream creation timeout");
      }
    }

    pa.pa_threaded_mainloop_unlock(m_mainloop);

    // cork means pausing, cork = 0 means resuming
    pa.pa_stream_cork(m_stream, 0, success_cb, this);

    return true;
  }

  bool stop() {
    if (!is_running()) {
      return false;
    }

    auto &pa = __pulseaudio_lib::instance();
    pa.pa_stream_unref(m_stream);
    m_stream = nullptr;
    pa.pa_context_unref(m_context);
    m_context = nullptr;
    pa.pa_threaded_mainloop_free(m_mainloop);
    m_mainloop = nullptr;
    m_api = nullptr;
    m_stream = nullptr;
    return true;
  }

  using buffer_size_t = uint32_t;

  buffer_size_t get_buffer_size_frames() const noexcept { return m_frames; }

  string_view name() const noexcept { return m_name; }

  using device_id_t = int;
  device_id_t device_id() const noexcept { return -1; }

  bool is_input() const noexcept { return true; }

  bool is_output() const noexcept { return true; }

  int get_num_input_channels() const noexcept { return 0; }

  int get_num_output_channels() const noexcept { return 2; }

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
    auto &pa = __pulseaudio_lib::instance();
    return m_stream && pa.pa_stream_is_corked(m_stream);
  }

  // you can put whatever samplerate you want with pulseaudio.
  span<const sample_rate_t> get_supported_sample_rates() const noexcept {
    static constexpr std::array<sample_rate_t, 2> srs{44100., 48000.};
    return {srs.data(), srs.size()};
  }

  //  you can put whatever buffersize you want with pulseaudio (and it won't generally be respected anyways)
  span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept {
    static constexpr std::array<buffer_size_t, 6> bs{32,  64,  128,
                                                     256, 512, 1024};
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
  static void output_callback(pa_stream *stream, size_t requested_bytes,
                              void *userdata) {
    auto &pa = __pulseaudio_lib::instance();
    auto &self = *static_cast<audio_basic_device *>(userdata);
    float *buffer = nullptr;
    size_t bytes_to_fill = requested_bytes;

    do {
      if (auto res = pa.pa_stream_begin_write(stream, (void **)&buffer,
                                              &bytes_to_fill);
          res != 0) {
        // we're in huge trouble
        return;
      }

      const auto size =
          bytes_to_fill / (sizeof(float) * self.get_num_output_channels());
      {
        if (self.m_callback) {
          audio_device_buffers bufs;
          // Pulseaudio entirely separates input and playback
          // so to have a full duplex callback, it needs to be buffered instead
          using buffer_t = audio_basic_buffer<
              float, std::experimental::audio_buffer_order_interleaved>;

          buffer_t outb((float *)buffer, size * self.get_num_output_channels(),
                        self.get_num_output_channels());
          bufs.__output_buffer = outb;

          self.m_callback(self, bufs);
        }
      }

      if (auto res = pa.pa_stream_write(stream, buffer, bytes_to_fill, nullptr,
                                        0LL, PA_SEEK_RELATIVE);
          res != 0) {
        // we're in huge trouble
        return;
      }

      requested_bytes -= bytes_to_fill;
      bytes_to_fill = requested_bytes;
    } while (requested_bytes > 0);
  }

  static void success_cb(pa_stream *, int, void *) {}

  std::string m_name;
  double m_sampleRate{};
  buffer_size_t m_frames{};

  pa_threaded_mainloop *m_mainloop{};
  pa_mainloop_api *m_api{};
  pa_context *m_context{};
  pa_stream *m_stream{};

  using __pulseaudio_callback_t =
      function<void(__pulseaudio_device &, audio_device_buffers &)>;
  __pulseaudio_callback_t m_callback;
};

template<>
class audio_basic_device_list<__pulseaudio_driver_t> : public forward_list<__pulseaudio_device>
{
public:
    audio_basic_device_list()
    {
        this->emplace_front(__pulseaudio_device{});
    }
};

template <>
auto get_default_audio_input_device<__pulseaudio_driver_t>()
    -> optional<audio_basic_device<__pulseaudio_driver_t>> {
  return __pulseaudio_device{};
}

template <>
auto get_default_audio_output_device<__pulseaudio_driver_t>()
    -> optional<audio_basic_device<__pulseaudio_driver_t>> {
  return __pulseaudio_device{};
}

template <>
auto get_audio_input_device_list<__pulseaudio_driver_t>()
    -> audio_basic_device_list<__pulseaudio_driver_t> {
  return {};
}

template <>
auto get_audio_output_device_list<__pulseaudio_driver_t>()
    -> audio_basic_device_list<__pulseaudio_driver_t> {
  return {};
}

_LIBSTDAUDIO_NAMESPACE_END
