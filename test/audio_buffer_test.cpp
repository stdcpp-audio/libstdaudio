// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE("Interleaved contiguous buffer") {
  std::array<float, 6> data = {0, 1, 2, 3, 4, 5};
  auto buffer = audio_buffer(data.data(), 3, 2, contiguous_interleaved);

  SECTION("data() pointer matches pointer passed in") {
    CHECK(buffer.data() == data.data());
  }

  SECTION("is_contiguous returns true") {
    CHECK(buffer.is_contiguous());
  }

  SECTION("frames_are_contiguous returns true") {
    CHECK(buffer.frames_are_contiguous());
  }

  SECTION("channels_are_contiguous returns false") {
    CHECK(!buffer.channels_are_contiguous());
  }

  SECTION("size_channels returns correct value") {
    CHECK(buffer.size_channels() == 2);
  }

  SECTION("size_frames returns correct value") {
    CHECK(buffer.size_frames() == 3);
  }

  SECTION("size_samples returns correct value") {
    CHECK(buffer.size_samples() == 6);
  }

  SECTION("Element read (const)") {
    const auto& cbuffer = buffer;
    CHECK(cbuffer(0, 0) == 0);
    CHECK(cbuffer(1, 0) == 2);
    CHECK(cbuffer(2, 0) == 4);
    CHECK(cbuffer(0, 1) == 1);
    CHECK(cbuffer(1, 1) == 3);
    CHECK(cbuffer(2, 1) == 5);
  }

  SECTION("Element write") {
    buffer(0, 0) = 6;
    buffer(1, 0) = 7;
    buffer(2, 0) = 8;
    buffer(0, 1) = 9;
    buffer(1, 1) = 10;
    buffer(2, 1) = 11;

    CHECK(data == std::array<float, 6>{6, 9, 7, 10, 8, 11});
  }
}

TEST_CASE("Deinterleaved contiguous buffer") {
  std::array<float, 6> data = {0, 1, 2, 3, 4, 5};
  auto buffer = audio_buffer(data.data(), 3, 2, contiguous_deinterleaved);

  SECTION("data() pointer matches pointer passed in") {
    CHECK(buffer.data() == data.data());
  }

  SECTION("Deinterleaved contiguous buffer satisfies is_contiguous() == true") {
    CHECK(buffer.is_contiguous());
  }

  SECTION("Deinterleaved contiguous buffer satisfies frames_are_contiguous() == false") {
    CHECK(!buffer.frames_are_contiguous());
  }

  SECTION("Deinterleaved contiguous buffer satisfies channels_are_contiguous() == true") {
    CHECK(buffer.channels_are_contiguous());
  }

  SECTION("size_channels returns correct value") {
    CHECK(buffer.size_channels() == 2);
  }

  SECTION("size_frames returns correct value") {
    CHECK(buffer.size_frames() == 3);
  }

  SECTION("size_samples returns correct value") {
    CHECK(buffer.size_samples() == 6);
  }

  SECTION("Element read (const)") {
    const auto& cbuffer = buffer;
    CHECK(cbuffer(0, 0) == 0);
    CHECK(cbuffer(1, 0) == 1);
    CHECK(cbuffer(2, 0) == 2);
    CHECK(cbuffer(0, 1) == 3);
    CHECK(cbuffer(1, 1) == 4);
    CHECK(cbuffer(2, 1) == 5);
  }

  SECTION("Element write") {
    buffer(0, 0) = 6;
    buffer(1, 0) = 7;
    buffer(2, 0) = 8;
    buffer(0, 1) = 9;
    buffer(1, 1) = 10;
    buffer(2, 1) = 11;

    CHECK(data == std::array<float, 6>{6, 7, 8, 9, 10, 11});
  }
}

TEST_CASE("Deinterleaved pointer-to-pointer  buffer") {
  std::array<float, 3> left = {0, 1, 2};
  std::array<float, 3> right = {3, 4, 5};
  std::array<float*, 2> data = {left.data(), right.data()};

  auto buffer = audio_buffer(data.data(), 3, 2, ptr_to_ptr_deinterleaved);

  SECTION("data() returns nullptr") {
    CHECK(buffer.data() == nullptr);
  }

  SECTION("is_contiguous returns false") {
    CHECK(!buffer.is_contiguous());
  }

  SECTION("frames_are_contiguous returns false") {
    CHECK(!buffer.frames_are_contiguous());
  }

  SECTION("channels_are_contiguous returns true") {
    CHECK(buffer.channels_are_contiguous());
  }

  SECTION("size_channels returns correct value") {
    CHECK(buffer.size_channels() == 2);
  }

  SECTION("size_frames returns correct value") {
    CHECK(buffer.size_frames() == 3);
  }

  SECTION("size_samples returns correct value") {
    CHECK(buffer.size_samples() == 6);
  }

  SECTION("Element read (const)") {
    const auto& cbuffer = buffer;
    CHECK(cbuffer(0, 0) == 0);
    CHECK(cbuffer(1, 0) == 1);
    CHECK(cbuffer(2, 0) == 2);
    CHECK(cbuffer(0, 1) == 3);
    CHECK(cbuffer(1, 1) == 4);
    CHECK(cbuffer(2, 1) == 5);
  }

  SECTION("Element write") {
    buffer(0, 0) = 6;
    buffer(1, 0) = 7;
    buffer(2, 0) = 8;
    buffer(0, 1) = 9;
    buffer(1, 1) = 10;
    buffer(2, 1) = 11;

    CHECK(left == std::array<float, 3>{6, 7, 8});
    CHECK(right == std::array<float, 3>{9, 10, 11});
  }
}