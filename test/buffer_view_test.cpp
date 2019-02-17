// libstdaudio
// Copyright (c) 2018 - Timur Doumler
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental::audio;

TEST_CASE( "Channel view type", "[buffer_view]") {
  buffer buf;
  REQUIRE(std::is_same_v<decltype(buf.channels()), channel_view>);
  REQUIRE(std::is_same_v<channel_view, buffer_view<buffer_view_type::channels>>);
}

TEST_CASE( "Frame view type", "[buffer_view]") {
  buffer buf;
  REQUIRE(std::is_same_v<decltype(buf.frames()), frame_view>);
  REQUIRE(std::is_same_v<frame_view, buffer_view<buffer_view_type::frames>>);
}

TEST_CASE( "Channel type", "[buffer_view]") {
  buffer buf;
  REQUIRE(std::is_same_v<decltype(buf.channels()[0]), strided_span<float>>);
}

TEST_CASE( "Frame type", "[buffer_view]") {
  buffer buf;
  REQUIRE(std::is_same_v<decltype(buf.frames()[0]), strided_span<float>>);
}

TEST_CASE( "Empty channel view size", "[buffer_view]") {
  buffer b;
  channel_view channels = b.channels();
  REQUIRE(channels.empty());
  REQUIRE(channels.size() == 0);
}

TEST_CASE( "Empty frame view size", "[buffer_view]") {
  buffer b;
  frame_view frames = b.frames();
  REQUIRE(frames.empty());
  REQUIRE(frames.size() == 0);
}

TEST_CASE( "Single-channel interleaved channel view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5};
  auto buf = buffer(data, 1, buffer_order::interleaved);

  auto channels = buf.channels();
  REQUIRE(!channels.empty());
  REQUIRE(channels.size() == 1);
}

TEST_CASE( "Single-channel interleaved frame view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5};
  auto buf = buffer(data, 1, buffer_order::interleaved);

  auto frames = buf.frames();
  REQUIRE(!frames.empty());
  REQUIRE(frames.size() == 5);
}

TEST_CASE( "Single-channel deinterleaved channel view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5};
  auto buf = buffer(data, 1, buffer_order::deinterleaved);

  auto channels = buf.channels();
  REQUIRE(!channels.empty());
  REQUIRE(channels.size() == 1);
}

TEST_CASE( "Single-channel deinterleaved frame view", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5};
  auto buf = buffer(data, 1, buffer_order::deinterleaved);

  auto frames = buf.frames();
  REQUIRE(!frames.empty());
  REQUIRE(frames.size() == 5);
}

TEST_CASE( "Multi-channel interleaved channel view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);

  auto channels = buf.channels();
  REQUIRE(!channels.empty());
  REQUIRE(channels.size() == 3);
}

TEST_CASE( "Multi-channel interleaved frame view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);

  auto frames = buf.frames();
  REQUIRE(!frames.empty());
  REQUIRE(frames.size() == 2);
}

TEST_CASE( "Multi-channel deinterleaved channel view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);

  auto channels = buf.channels();
  REQUIRE(!channels.empty());
  REQUIRE(channels.size() == 3);
}

TEST_CASE( "Multi-channel deinterleaved frame view size", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);

  auto frames = buf.frames();
  REQUIRE(!frames.empty());
  REQUIRE(frames.size() == 2);
}

TEST_CASE( "Channel view element type", "[buffer_view]") {
  buffer buf;
  REQUIRE(std::is_same_v<decltype(buf.channels()[0]), strided_span<float>>);
}

TEST_CASE( "Frame view element type", "[buffer_view]") {
  buffer buf;
  REQUIRE(std::is_same_v<decltype(buf.frames()[0]), strided_span<float>>);
}

TEST_CASE( "Interleaved channel view elements", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);

  auto channels = buf.channels();
  REQUIRE(channels[0].size() == 2);
  REQUIRE(channels[0][0] == 1);
  REQUIRE(channels[0][1] == 4);

  REQUIRE(channels[1].size() == 2);
  REQUIRE(channels[1][0] == 2);
  REQUIRE(channels[1][1] == 5);

  REQUIRE(channels[2].size() == 2);
  REQUIRE(channels[2][0] == 3);
  REQUIRE(channels[2][1] == 6);
}

TEST_CASE( "Interleaved frame view elements", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);

  auto frames = buf.frames();
  REQUIRE(frames[0].size() == 3);
  REQUIRE(frames[0][0] == 1);
  REQUIRE(frames[0][1] == 2);
  REQUIRE(frames[0][2] == 3);

  REQUIRE(frames[1].size() == 3);
  REQUIRE(frames[1][0] == 4);
  REQUIRE(frames[1][1] == 5);
  REQUIRE(frames[1][2] == 6);
}

TEST_CASE( "Deinterleaved channel view elements", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);

  auto channels = buf.channels();
  REQUIRE(channels[0].size() == 2);
  REQUIRE(channels[0][0] == 1);
  REQUIRE(channels[0][1] == 2);

  REQUIRE(channels[1].size() == 2);
  REQUIRE(channels[1][0] == 3);
  REQUIRE(channels[1][1] == 4);

  REQUIRE(channels[2].size() == 2);
  REQUIRE(channels[2][0] == 5);
  REQUIRE(channels[2][1] == 6);
}

TEST_CASE( "Deinterleaved frame view elements", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);

  auto frames = buf.frames();
  REQUIRE(frames[0].size() == 3);
  REQUIRE(frames[0][0] == 1);
  REQUIRE(frames[0][1] == 3);
  REQUIRE(frames[0][2] == 5);

  REQUIRE(frames[1].size() == 3);
  REQUIRE(frames[1][0] == 2);
  REQUIRE(frames[1][1] == 4);
  REQUIRE(frames[1][2] == 6);
}

TEST_CASE( "Empty channel view begin()/end() comparison", "[buffer_view]") {
  buffer buf;
  auto channels = buf.channels();

  REQUIRE(channels.begin() == channels.end());
  REQUIRE_FALSE(channels.begin() != channels.end());
  REQUIRE_FALSE(channels.begin() < channels.end());
  REQUIRE(channels.begin() <= channels.end());
  REQUIRE_FALSE(channels.begin() > channels.end());
  REQUIRE(channels.begin() >= channels.end());

  REQUIRE(channels.end() == channels.begin());
  REQUIRE_FALSE(channels.end() != channels.begin());
  REQUIRE_FALSE(channels.end() < channels.begin());
  REQUIRE(channels.end() <= channels.begin());
  REQUIRE_FALSE(channels.end() > channels.begin());
  REQUIRE(channels.end() >= channels.begin());
}

TEST_CASE( "Empty frame view begin()/end() comparison", "[buffer_view]") {
  buffer buf;
  auto frames = buf.frames();

  REQUIRE(frames.begin() == frames.end());
  REQUIRE_FALSE(frames.begin() != frames.end());
  REQUIRE_FALSE(frames.begin() < frames.end());
  REQUIRE(frames.begin() <= frames.end());
  REQUIRE_FALSE(frames.begin() > frames.end());
  REQUIRE(frames.begin() >= frames.end());

  REQUIRE(frames.end() == frames.begin());
  REQUIRE_FALSE(frames.end() != frames.begin());
  REQUIRE_FALSE(frames.end() < frames.begin());
  REQUIRE(frames.end() <= frames.begin());
  REQUIRE_FALSE(frames.end() > frames.begin());
  REQUIRE(frames.end() >= frames.begin());
}

TEST_CASE( "Empty channel view range-based for loop", "[buffer_view]") {
  buffer buf;
  for (auto& channel : buf.channels())
    REQUIRE(false); // should never enter this loop
}

TEST_CASE( "Empty frame view range-based for loop", "[buffer_view]") {
  buffer buf;
  for (auto& frame : buf.frames())
    REQUIRE(false); // should never enter this loop
}

TEST_CASE( "Non-empty channel view begin()/end() comparison", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);
  auto channels = buf.channels();

  REQUIRE_FALSE(channels.begin() == channels.end());
  REQUIRE(channels.begin() != channels.end());
  REQUIRE(channels.begin() < channels.end());
  REQUIRE(channels.begin() <= channels.end());
  REQUIRE_FALSE(channels.begin() > channels.end());
  REQUIRE_FALSE(channels.begin() >= channels.end());

  REQUIRE_FALSE(channels.end() == channels.begin());
  REQUIRE(channels.end() != channels.begin());
  REQUIRE_FALSE(channels.end() < channels.begin());
  REQUIRE_FALSE(channels.end() <= channels.begin());
  REQUIRE(channels.end() > channels.begin());
  REQUIRE(channels.end() >= channels.begin());
}

TEST_CASE( "Non-empty frame view begin()/end() comparison", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);
  auto frames = buf.frames();

  REQUIRE_FALSE(frames.begin() == frames.end());
  REQUIRE(frames.begin() != frames.end());
  REQUIRE(frames.begin() < frames.end());
  REQUIRE(frames.begin() <= frames.end());
  REQUIRE_FALSE(frames.begin() > frames.end());
  REQUIRE_FALSE(frames.begin() >= frames.end());

  REQUIRE_FALSE(frames.end() == frames.begin());
  REQUIRE(frames.end() != frames.begin());
  REQUIRE_FALSE(frames.end() < frames.begin());
  REQUIRE_FALSE(frames.end() <= frames.begin());
  REQUIRE(frames.end() > frames.begin());
  REQUIRE(frames.end() >= frames.begin());
}

TEST_CASE( "Interleaved channels iterator dereference", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);
  auto channels = buf.channels();
  auto it = channels.begin();

  REQUIRE(it->size() == 2);
  REQUIRE((*it)[0] == 1);
  REQUIRE((*it)[1] == 4);
}

TEST_CASE( "Denterleaved channels iterator dereference", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);
  auto channels = buf.channels();
  auto it = channels.begin();

  REQUIRE(it->size() == 2);
  REQUIRE((*it)[0] == 1);
  REQUIRE((*it)[1] == 2);
}

TEST_CASE( "Interleaved frames iterator dereference", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::interleaved);
  auto frames = buf.frames();
  auto it = frames.begin();

  REQUIRE(it->size() == 3);
  REQUIRE((*it)[0] == 1);
  REQUIRE((*it)[1] == 2);
  REQUIRE((*it)[2] == 3);
}

TEST_CASE( "Denterleaved frames iterator dereference", "[buffer_view]") {
  float data[] = {1, 2, 3, 4, 5, 6};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);
  auto frames = buf.frames();
  auto it = frames.begin();

  REQUIRE(it->size() == 3);
  REQUIRE((*it)[0] == 1);
  REQUIRE((*it)[1] == 3);
  REQUIRE((*it)[2] == 5);
}

TEST_CASE( "Interleaved channels iterator prefix-increment", "[buffer_view]") {
  float data[] = {1, 2, 3};
  auto buf = buffer(data, 1, buffer_order::interleaved);
  auto channels = buf.channels();
  auto it = channels.begin();

  REQUIRE(it != channels.end());
  REQUIRE(++it == channels.end());
  REQUIRE(it == channels.end());
}

TEST_CASE( "Deinterleaved channels iterator prefix-increment", "[buffer_view]") {
  float data[] = {1, 2, 3};
  auto buf = buffer(data, 1, buffer_order::deinterleaved);
  auto channels = buf.channels();
  auto it = channels.begin();

  REQUIRE(it != channels.end());
  REQUIRE(++it == channels.end());
  REQUIRE(it == channels.end());
}

TEST_CASE( "Interleaved frames iterator prefix-increment", "[buffer_view]") {
  float data[] = {1, 2, 3};
  auto buf = buffer(data, 3, buffer_order::interleaved);
  auto frames = buf.frames();
  auto it = frames.begin();

  REQUIRE(it != frames.end());
  REQUIRE(++it == frames.end());
  REQUIRE(it == frames.end());
}

TEST_CASE( "Deinterleaved frames iterator prefix-increment", "[buffer_view]") {
  float data[] = {1, 2, 3};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);
  auto frames = buf.frames();
  auto it = frames.begin();

  REQUIRE(it != frames.end());
  REQUIRE(++it == frames.end());
  REQUIRE(it == frames.end());
}

TEST_CASE( "Interleaved channels iterator postfix-increment", "[buffer_view]") {
  float data[] = {1, 2, 3};
  auto buf = buffer(data, 1, buffer_order::interleaved);
  auto channels = buf.channels();
  auto it = channels.begin();

  REQUIRE(it != channels.end());
  REQUIRE(it++ != channels.end());
  REQUIRE(it == channels.end());
}

TEST_CASE( "Deinterleaved channels iterator postfix-increment", "[buffer_view]") {
  float data[] = {1, 2, 3};
  auto buf = buffer(data, 1, buffer_order::deinterleaved);
  auto channels = buf.channels();
  auto it = channels.begin();

  REQUIRE(it != channels.end());
  REQUIRE(it++ != channels.end());
  REQUIRE(it == channels.end());
}

TEST_CASE( "Interleaved frames iterator postfix-increment", "[buffer_view]") {
  float data[] = {1, 2, 3};
  auto buf = buffer(data, 3, buffer_order::interleaved);
  auto frames = buf.frames();
  auto it = frames.begin();

  REQUIRE(it != frames.end());
  REQUIRE(it++ != frames.end());
  REQUIRE(it == frames.end());
}

TEST_CASE( "Deinterleaved frames iterator postfix-increment", "[buffer_view]") {
  float data[] = {1, 2, 3};
  auto buf = buffer(data, 3, buffer_order::deinterleaved);
  auto frames = buf.frames();
  auto it = frames.begin();

  REQUIRE(it != frames.end());
  REQUIRE(it++ != frames.end());
  REQUIRE(it == frames.end());
}

TEST_CASE( "Buffer channels increment/decrement", "[buffer]") {
  float more_data[] = {0, 1, 0, -1, 0, 2, 0, -2};
  auto buf = buffer(more_data, 4, buffer_order::interleaved);
  auto b = buf.channels().begin();
  auto b_orig = b;
  auto e = buf.channels().end();

  ++b;
  b++;
  b+=2;

  REQUIRE(b == e);
  REQUIRE(b != b_orig);

  --b;
  b--;
  b-=2;

  REQUIRE(b != e);
  REQUIRE(b == b_orig);

  b = b + 4;

  REQUIRE(b == e);
  REQUIRE(b != b_orig);

  b = b - 4;

  REQUIRE(b != e);
  REQUIRE(b == b_orig);
}

TEST_CASE( "Buffer channels relational operators", "[buffer]") {
  float more_data[] = {0, 1, 0, -1, 0, 2, 0, -2};
  auto buf = buffer(more_data, 4, buffer_order::interleaved);
  auto b = buf.channels().begin();
  auto b_orig = b;

  REQUIRE(!(b > b_orig));
  REQUIRE(!(b < b_orig));
  REQUIRE(!(b_orig > b));
  REQUIRE(!(b_orig < b));
  REQUIRE(b <= b_orig);
  REQUIRE(b >= b_orig);
  REQUIRE(b_orig <= b);
  REQUIRE(b_orig >= b);

  ++b;

  REQUIRE(b > b_orig);
  REQUIRE(!(b < b_orig));
  REQUIRE(b_orig < b);
  REQUIRE(!(b_orig > b));
  REQUIRE(!(b <= b_orig));
  REQUIRE(b >= b_orig);
  REQUIRE(b_orig <= b);
  REQUIRE(!(b_orig >= b));
}

TEST_CASE( "Buffer channels subscript operator", "[buffer]") {
  float more_data[] = {0, 1, 0, -1, 0, 2, 0, -2};
  auto buf = buffer(more_data, 4, buffer_order::interleaved);
  auto b = buf.channels().begin();

  auto elem0 = buf.channels()[0];
  auto elem1 = buf.channels()[1];
  auto elem2 = buf.channels()[2];
  auto elem3 = buf.channels()[3];

  REQUIRE(*b == elem0);
  REQUIRE(*(++b) == elem1);
  REQUIRE(*(++b) == elem2);
  REQUIRE(*(++b) == elem3);
}
