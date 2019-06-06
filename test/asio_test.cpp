// libstdaudio
// Copyright (c) 2019 - Andy Saul
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include <array>
#include "catch/catch.hpp"
#include "catch/trompeloeil.hpp"
#include "trompeloeil.hpp"

using namespace std::experimental;

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
