// libstdaudio
// Copyright (c) 2019 - Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include "catch/catch.hpp"
#include "catch/trompeloeil.hpp"
#include "trompeloeil.hpp"

using namespace std::experimental;

using trompeloeil::_;

using sample_rate_t = audio_device::sample_rate_t;
using buffer_size_t = audio_device::buffer_size_t;

TEST_CASE("Converts floating point samples to integer samples", "[asio]")
{
  SECTION("Converts to 32-bit samples")
  {
    CHECK(__asio_sample<int32_t>(0.f).int_value() == 0);
    CHECK(__asio_sample<int32_t>(1.f).int_value() == 0x7fff'ffff);
    CHECK(__asio_sample<int32_t>(-1.f).int_value() == -0x7fff'ffff);
    CHECK(__asio_sample<int32_t>(0.5f).int_value() == 0x7fff'ffff / 2);
    CHECK(__asio_sample<int32_t>(-0.5f).int_value() == -0x7fff'ffff / 2);
  }

  SECTION("Converts to 24-bit samples")
  {
    CHECK(__asio_sample<packed24_t>(0.f).int_value() == 0);
    CHECK(__asio_sample<packed24_t>(1.f).int_value() == 0x7f'ffff);
    CHECK(__asio_sample<packed24_t>(-1.f).int_value() == -0x7f'ffff);
    CHECK(__asio_sample<packed24_t>(0.5f).int_value() == 0x7f'ffff / 2);
    CHECK(__asio_sample<packed24_t>(-0.5f).int_value() == -0x7f'ffff / 2);
  }

  SECTION("Converts to 16-bit samples")
  {
    CHECK(__asio_sample<int16_t>(0.f).int_value() == 0);
    CHECK(__asio_sample<int16_t>(1.f).int_value() == 0x7fff);
    CHECK(__asio_sample<int16_t>(-1.f).int_value() == -0x7fff);
    CHECK(__asio_sample<int16_t>(0.5f).int_value() == 0x7fff / 2);
    CHECK(__asio_sample<int16_t>(-0.5f).int_value() == -0x7fff / 2);
  }
}

TEST_CASE("Converts integer samples to floating point samples", "[asio]")
{
  SECTION("Converts from 32-bit samples")
  {
    CHECK(__asio_sample<int32_t>(0).float_value() == 0.f);
    CHECK(__asio_sample<int32_t>(0x7fff'ffff).float_value() == 1.f);
    CHECK(__asio_sample<int32_t>(-0x7fff'ffff).float_value() == -1.f);
    CHECK(__asio_sample<int32_t>(0x7fff'ffff / 2).float_value() == 0.5f);
    CHECK(__asio_sample<int32_t>(-0x7fff'ffff / 2).float_value() == -0.5f);
  }

  SECTION("Converts from 24-bit samples")
  {
    CHECK(__asio_sample<packed24_t>(0).float_value() == 0.f);;
    CHECK(__asio_sample<packed24_t>(0x7f'ffff).float_value() == 1.f);
    CHECK(__asio_sample<packed24_t>(-0x7f'ffff).float_value() == -1.f);

    const auto half = __asio_sample<packed24_t>(0x7f'ffff / 2).float_value();
    CHECK(Approx(half) == 0.5f);

    const auto minus_half = __asio_sample<packed24_t>(-0x7f'ffff / 2).float_value();
    CHECK(Approx(minus_half) == -0.5f);
  }

  SECTION("Converts from 16-bit samples")
  {
    int16_t value = 0;
    CHECK(__asio_sample<int16_t>(value).float_value() == 0.f);

    value = 0x7fff;
    CHECK(__asio_sample<int16_t>(value).float_value() == 1.f);

    value = -0x7fff;
    CHECK(__asio_sample<int16_t>(value).float_value() == -1.f);

    value = 0x7fff / 2;
    const auto half = __asio_sample<int16_t>(value).float_value();
    CHECK(Approx(half).epsilon(0.0001f) == 0.5f);

    value = -0x7fff / 2;
    const auto minus_half = __asio_sample<int16_t>(value).float_value();
    CHECK(Approx(minus_half).epsilon(0.0001f) == -0.5f);
  }
}

class mock_asio : public trompeloeil::mock_interface<IASIO>
{
public:
  MAKE_MOCK2(QueryInterface, long(const IID&, void**), override);
  IMPLEMENT_MOCK0(AddRef);
  IMPLEMENT_MOCK0(Release);
  IMPLEMENT_MOCK1(init);
  IMPLEMENT_MOCK1(getDriverName);
  IMPLEMENT_MOCK0(getDriverVersion);
  IMPLEMENT_MOCK1(getErrorMessage);
  IMPLEMENT_MOCK0(start);
  IMPLEMENT_MOCK0(stop);
  IMPLEMENT_MOCK2(getChannels);
  IMPLEMENT_MOCK2(getLatencies);
  IMPLEMENT_MOCK4(getBufferSize);
  IMPLEMENT_MOCK1(canSampleRate);
  IMPLEMENT_MOCK1(getSampleRate);
  IMPLEMENT_MOCK1(setSampleRate);
  IMPLEMENT_MOCK2(getClockSources);
  IMPLEMENT_MOCK1(setClockSource);
  IMPLEMENT_MOCK2(getSamplePosition);
  IMPLEMENT_MOCK1(getChannelInfo);
  IMPLEMENT_MOCK4(createBuffers);
  IMPLEMENT_MOCK0(disposeBuffers);
  IMPLEMENT_MOCK0(controlPanel);
  IMPLEMENT_MOCK2(future);
  IMPLEMENT_MOCK0(outputReady);
};

class asio_device_builder
{
public:
  asio_device_builder(mock_asio& asio) : asio(asio)
  {}

  using audio_device_ptr = std::unique_ptr<audio_device, std::function<void(audio_device*)>>;

  audio_device_ptr operator()()
  {
    REQUIRE_CALL(asio, AddRef()).RETURN(1);

    REQUIRE_CALL(asio, getChannels(_, _))
      .SIDE_EFFECT(*_1 = num_inputs)
      .SIDE_EFFECT(*_2 = num_outputs)
      .RETURN(0);

    REQUIRE_CALL(asio, getBufferSize(_, _, _, _))
      .SIDE_EFFECT(*_1 = min_buffer_size)
      .SIDE_EFFECT(*_2 = max_buffer_size)
      .SIDE_EFFECT(*_3 = preferred_buffer_size)
      .SIDE_EFFECT(*_4 = granularity)
      .RETURN(0);

    REQUIRE_CALL(asio, getChannelInfo(_))
      .SIDE_EFFECT(*_1 = info)
      .RETURN(0);

    REQUIRE_CALL(asio, canSampleRate(_))
      .TIMES(6)
      .RETURN(is_supported_sample_rate(_1));

    constexpr double sample_rate{ 44'100.0 };
    REQUIRE_CALL(asio, getSampleRate(_))
      .SIDE_EFFECT(*_1 = sample_rate)
      .TIMES(AT_LEAST(1))
      .RETURN(ASIOTrue);

    return {new audio_device(id, name, &asio, direction), [&](auto* device) {
      REQUIRE_CALL(asio, stop()).RETURN(ASIOTrue);
      REQUIRE_CALL(asio, Release()).RETURN(0);
      ALLOW_CALL(asio, disposeBuffers()).RETURN(0);
      delete device;
    }};
  }

  long is_supported_sample_rate(const sample_rate_t rate) const {
    return std::find(sample_rates.begin(), sample_rates.end(), rate) == sample_rates.end()
      ? ASE_NotPresent
      : ASE_OK;
  }

  asio_device_builder& with_id(const audio_device::device_id_t new_id) {
    id = new_id;
    return *this;
  }

  asio_device_builder& with_name(const std::string& new_name) {
    name = new_name;
    return *this;
  }

  asio_device_builder& with_num_input_channels(const long new_num_inputs) {
    num_inputs = new_num_inputs;
    return *this;
  }

  asio_device_builder& with_num_output_channels(const long new_num_outputs) {
    num_outputs = new_num_outputs;
    return *this;
  }

  asio_device_builder& using_inputs_only() {
    direction = audio_direction::in;
    return *this;
  }

  asio_device_builder& using_outputs_only() {
    direction = audio_direction::out;
    return *this;
  }

  asio_device_builder& with_unique_buffer_size(const long buffer_size) {
    min_buffer_size = max_buffer_size = preferred_buffer_size = buffer_size;
    granularity = 0;
    return *this;
  }

  asio_device_builder& with_power_of_two_buffer_sizes(const long min, const long max) {
    min_buffer_size = min;
    max_buffer_size = max;
    granularity = -1;
    return *this;
  }

  asio_device_builder& with_preferred_buffer_size(const long preferred) {
    preferred_buffer_size = preferred;
    return *this;
  }

  asio_device_builder& with_buffer_size_range(const long min, const long max, const long step) {
    min_buffer_size = min;
    max_buffer_size = max;
    granularity = step;
    return *this;
  }

  asio_device_builder& with_sample_rates(std::vector<sample_rate_t> new_sample_rates) {
    sample_rates = new_sample_rates;
    return *this;
  }

private:
  mock_asio& asio;
  audio_device::device_id_t id{0};
  std::string name;
  audio_direction direction{audio_direction::full_duplex};

  long num_inputs{0};
  long num_outputs{0};

  long min_buffer_size{128};
  long max_buffer_size{512};
  long preferred_buffer_size{256};
  long granularity{-1};
  ASIOChannelInfo info{0, ASIOFalse, ASIOFalse, 0, ASIOSTInt32LSB};

  std::vector<sample_rate_t> sample_rates{44'100, 48'000, 88'200, 96'000, 176'400, 192'000};
};

class asio_device_fixture
{
public:
  mock_asio asio;
  asio_device_builder make_asio_device{asio};

  ASIOCallbacks* callbacks;
  ASIOBufferInfo* asio_buffers;
  std::vector<int32_t> buffer;

  template <typename _CallbackType>
  void connect(audio_device& device, _CallbackType callback) {
    REQUIRE_CALL(asio, createBuffers(_, _, _, _))
      .LR_SIDE_EFFECT(asio_buffers = _1)
      .LR_SIDE_EFFECT(callbacks = _4)
      .RETURN(0);
    device.connect(callback);
  }

  void allocate_buffers(const uint32_t num_channels, const uint32_t num_frames) {
    buffer.resize(2 * num_channels * num_frames);
    auto p = buffer.data();
    for (uint32_t c = 0; c < num_channels; ++c) {
      for (uint32_t i = 0; i < 2; ++i) {
        asio_buffers[c].buffers[i] = p;
        p += num_frames;
      }
    }
  }

  void verify_out_value(const uint32_t channel, const int32_t value) const {
    CHECK(*static_cast<int32_t*>(asio_buffers[channel].buffers[0]) == value);
  }

  void set_input_value(const int channel, const int32_t value) const {
    auto samples = static_cast<int32_t*>(asio_buffers[channel].buffers[0]);
    samples[0] = value;
    samples = static_cast<int32_t*>(asio_buffers[channel].buffers[1]);
    samples[0] = value;
  }
};

TEST_CASE_METHOD(asio_device_fixture, "Creates device with id", "[asio]")
{
  auto device = make_asio_device.with_id(42)();

  CHECK(device->device_id() == 42);
}

TEST_CASE_METHOD(asio_device_fixture, "Creates device with name", "[asio]")
{
  auto device = make_asio_device.with_name("Fake ASIO")();

  CHECK(device->name() == "Fake ASIO");
}

TEST_CASE_METHOD(asio_device_fixture, "Creates stereo output device", "[asio]")
{
  auto device = make_asio_device.with_num_output_channels(2)();

  CHECK(device->is_output());
  CHECK(device->get_num_output_channels() == 2);
}

TEST_CASE_METHOD(asio_device_fixture, "Creates mono output device", "[asio]")
{
  auto device = make_asio_device.with_num_output_channels(1)();

  CHECK(device->is_output());
  CHECK(device->get_num_output_channels() == 1);
}

TEST_CASE_METHOD(asio_device_fixture, "Creates stereo input device", "[asio]")
{
  auto device = make_asio_device.with_num_input_channels(2)();

  CHECK(device->is_input());
  CHECK(device->get_num_input_channels() == 2);
}

TEST_CASE_METHOD(asio_device_fixture, "Creates mono input device", "[asio]")
{
  auto device = make_asio_device.with_num_input_channels(1)();

  CHECK(device->is_input());
  CHECK(device->get_num_input_channels() == 1);
}

TEST_CASE_METHOD(asio_device_fixture, "Creates full duplex device", "[asio]")
{
  auto device = make_asio_device
    .with_num_input_channels(4)
    .with_num_output_channels(4)();

  CHECK(device->is_input());
  CHECK(device->get_num_input_channels() == 4);
  CHECK(device->is_output());
  CHECK(device->get_num_output_channels() == 4);
}

TEST_CASE_METHOD(asio_device_fixture, "Creates input-only device from duplex ASIO", "[asio]")
{
  auto device = make_asio_device
    .with_num_input_channels(4)
    .with_num_output_channels(4)
    .using_inputs_only()();

  CHECK(device->is_input());
  CHECK(device->get_num_input_channels() == 4);
  CHECK_FALSE(device->is_output());
  CHECK(device->get_num_output_channels() == 0);
}

TEST_CASE_METHOD(asio_device_fixture, "Creates output-only device from duplex ASIO", "[asio]")
{
  auto device = make_asio_device
    .with_num_input_channels(4)
    .with_num_output_channels(4)
    .using_outputs_only()();

  CHECK_FALSE(device->is_input());
  CHECK(device->get_num_input_channels() == 0);
  CHECK(device->is_output());
  CHECK(device->get_num_output_channels() == 4);
}

TEST_CASE_METHOD(asio_device_fixture, "Supports unique buffer size", "[asio]")
{
  auto device = make_asio_device.with_unique_buffer_size(1024)();

  auto buffer_sizes = device->get_supported_buffer_sizes_frames();

  CHECK(buffer_sizes.size() == 1);
  CHECK(buffer_sizes[0] == 1024);
  CHECK(device->get_buffer_size_frames() == 1024);
  CHECK(device->set_buffer_size_frames(1024));
  CHECK_FALSE(device->set_buffer_size_frames(64));
}

TEST_CASE_METHOD(asio_device_fixture, "Supports power-of-two buffer sizes", "[asio]")
{
  const std::array<buffer_size_t, 5> expected_buffer_sizes{64, 128, 256, 512, 1024};

  auto device = make_asio_device
    .with_power_of_two_buffer_sizes(64, 1024)
    .with_preferred_buffer_size(256)();

  const auto buffer_sizes = device->get_supported_buffer_sizes_frames();
  CHECK(buffer_sizes == span<const buffer_size_t>(expected_buffer_sizes));
  CHECK(device->get_buffer_size_frames() == 256);

  for (const auto size : expected_buffer_sizes) {
    CHECK(device->set_buffer_size_frames(size));
    CHECK_FALSE(device->set_buffer_size_frames(size + 1));
    CHECK_FALSE(device->set_buffer_size_frames(size - 1));
    CHECK(device->get_buffer_size_frames() == size);
  }
}

TEST_CASE_METHOD(asio_device_fixture, "Supports buffer size ranges", "[asio]")
{
  const std::array<buffer_size_t, 4> expected_buffer_sizes{64, 128, 192, 256};

  auto device = make_asio_device
    .with_buffer_size_range(64, 256, 64)
    .with_preferred_buffer_size(192)();

  const auto buffer_sizes = device->get_supported_buffer_sizes_frames();
  CHECK(buffer_sizes == span<const buffer_size_t>(expected_buffer_sizes));
  CHECK(device->get_buffer_size_frames() == 192);

  for (const auto size : expected_buffer_sizes) {
    CHECK(device->set_buffer_size_frames(size));
    CHECK_FALSE(device->set_buffer_size_frames(size + 1));
    CHECK_FALSE(device->set_buffer_size_frames(size - 1));
    CHECK(device->get_buffer_size_frames() == size);
  }
}

TEST_CASE_METHOD(asio_device_fixture, "Supports common samplerates", "[asio]")
{
  const std::array<sample_rate_t, 6>
    expected_sample_rates{44'100, 48'000, 88'200, 96'000, 176'400, 192'000};

  auto device = make_asio_device();

  const auto sample_rates = device->get_supported_sample_rates();
  CHECK(sample_rates == span<const sample_rate_t>(expected_sample_rates));

  for (const auto rate : expected_sample_rates) {
    ALLOW_CALL(asio, setSampleRate(_)).RETURN(make_asio_device.is_supported_sample_rate(rate));
    CHECK(device->set_sample_rate(rate));
  }
}

TEST_CASE_METHOD(asio_device_fixture, "Supports subset of common samplerates", "[asio]")
{
  const std::vector<sample_rate_t> expected_sample_rates{44'100, 48'000};

  auto device = make_asio_device.with_sample_rates(expected_sample_rates)();

  const auto sample_rates = device->get_supported_sample_rates();
  CHECK(sample_rates == span<const sample_rate_t>(expected_sample_rates));

  const std::array<sample_rate_t, 4> banned_sample_rates{88'200, 96'000, 176'400, 192'000};
  for (const auto rate : banned_sample_rates) {
    ALLOW_CALL(asio, setSampleRate(_)).RETURN(make_asio_device.is_supported_sample_rate(rate));
    CHECK_FALSE(device->set_sample_rate(rate));
  }
}

TEST_CASE_METHOD(asio_device_fixture, "Can connect to device", "[asio]")
{
  auto device = make_asio_device();

  CHECK(device->can_connect());
}

TEST_CASE_METHOD(asio_device_fixture, "Cannot poll device", "[asio]")
{
  auto device = make_asio_device();

  CHECK_FALSE(device->can_process());
}

TEST_CASE_METHOD(asio_device_fixture, "Device is not running before connection", "[asio]")
{
  auto device = make_asio_device();

  CHECK_FALSE(device->is_running());
}

TEST_CASE_METHOD(asio_device_fixture, "Device cannot be started before connection", "[asio]")
{
  auto device = make_asio_device();

  CHECK_FALSE(device->start());
}

TEST_CASE_METHOD(asio_device_fixture, "Device writes outputs from legacy callbacks", "[asio]")
{
  constexpr long num_channels{2};
  constexpr long num_frames{32};
  auto device = make_asio_device
    .with_num_output_channels(num_channels)
    .with_unique_buffer_size(num_frames)();

  connect(*device, [&](audio_device& d, audio_device_io<float>& io) mutable noexcept {
    CHECK(device.get() == &d);
    auto& out = *io.output_buffer;
    CHECK(io.output_buffer.has_value());
    CHECK(out.size_channels() == num_channels);
    CHECK(out.size_frames() == num_frames);
    for (int frame = 0; frame < out.size_frames(); ++frame) {
      out(frame, 0) = 1.0f;
      out(frame, 1) = -1.0f;
    }
  });

  allocate_buffers(num_channels, num_frames);

  REQUIRE_CALL(asio, start()).RETURN(0);
  REQUIRE_CALL(asio, outputReady()).RETURN(0);
  CHECK(device->start());

  callbacks->bufferSwitch(0, ASIOFalse);

  verify_out_value(0, 0x7fff'ffff);
  verify_out_value(1, -0x7fff'ffff);
}

TEST_CASE_METHOD(asio_device_fixture, "Device writes outputs from timestamped callbacks", "[asio]")
{
  constexpr long num_channels{2};
  constexpr long num_frames{32};
  auto device = make_asio_device
    .with_num_output_channels(num_channels)
    .with_unique_buffer_size(num_frames)();

  connect(*device, [&](audio_device& d, audio_device_io<float>& io) mutable noexcept {
    CHECK(device.get() == &d);
    auto& out = *io.output_buffer;
    CHECK(io.output_buffer.has_value());
    CHECK(out.size_channels() == num_channels);
    CHECK(out.size_frames() == num_frames);
    for (int frame = 0; frame < out.size_frames(); ++frame) {
      out(frame, 0) = 1.0f;
      out(frame, 1) = -1.0f;
    }
  });

  allocate_buffers(num_channels, num_frames);

  REQUIRE_CALL(asio, start()).RETURN(0);
  CHECK(device->start());

  ASIOTime time;
  auto result = callbacks->bufferSwitchTimeInfo(&time, 0, ASIOFalse);
  CHECK(result->timeInfo.samplePosition == 0);

  result = callbacks->bufferSwitchTimeInfo(&time, 0, ASIOFalse);
  CHECK(result->timeInfo.samplePosition == num_frames);

  verify_out_value(0, 0x7fff'ffff);
  verify_out_value(1, -0x7fff'ffff);
}

TEST_CASE_METHOD(asio_device_fixture, "Device responds to ASIO message callback", "[asio]")
{
  const auto device = make_asio_device();
  connect(*device, [](auto&, auto&) noexcept {});

  CHECK(callbacks->asioMessage(kAsioSupportsTimeInfo, 0, nullptr, nullptr) == ASIOTrue);
  CHECK(callbacks->asioMessage(kAsioEngineVersion, 0, nullptr, nullptr) == 2);
  CHECK(callbacks->asioMessage(kAsioSelectorSupported, 0, nullptr, nullptr) == ASIOFalse);
}

TEST_CASE_METHOD(asio_device_fixture, "Throws exception if connection attempted when already running", "[asio]")
{
  const auto device = make_asio_device
    .with_num_output_channels(1)
    .with_unique_buffer_size(1)();
  const auto no_op = [](auto&, auto&) noexcept {};
  connect(*device, no_op);
  REQUIRE_CALL(asio, start()).RETURN(0);
  CHECK(device->start());

  CHECK(device->is_running());
  CHECK_THROWS_AS(device->connect(no_op), audio_device_exception);
}

TEST_CASE_METHOD(asio_device_fixture, "Device reads inputs from legacy callbacks", "[asio]")
{
  constexpr long num_channels{ 2 };
  constexpr long num_frames{ 32 };
  auto device = make_asio_device
    .with_num_input_channels(num_channels)
    .with_unique_buffer_size(num_frames)();

  connect(*device, [&](audio_device& d, audio_device_io<float>& io) mutable noexcept {
    CHECK(device.get() == &d);
    auto& in = *io.input_buffer;
    CHECK(io.input_buffer.has_value());
    CHECK(in.size_channels() == num_channels);
    CHECK(in.size_frames() == num_frames);
    CHECK(in(0, 0) == 1.f);
    CHECK(in(0, 1) == -1.0f);
  });

  allocate_buffers(num_channels, num_frames);
  set_input_value(0, 0x7fff'ffff);
  set_input_value(1, -0x7fff'ffff);

  REQUIRE_CALL(asio, start()).RETURN(0);
  CHECK(device->start());

  REQUIRE_CALL(asio, outputReady()).RETURN(0);
  callbacks->bufferSwitch(0, ASIOFalse);
}
