// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Buffer default constructor", "[buffer]") {
  float data[] = {0, 1, 0, -1};
  auto buf = audio::buffer();
  REQUIRE(buf.raw().data() == nullptr);
  REQUIRE(buf.raw().size() == 0);

  // TODO: is this the correct default? Or should we have buffer_ordering::none?
  // On the other hand, none would create a new class invariant and a new constructor
  // precondition (ordering shall be none if no data, and non-none if there is data)
  REQUIRE(buf.get_ordering() == audio::buffer_ordering::interleaved   );
}

TEST_CASE( "Buffer empty()", "[buffer]") {
  auto buf = audio::buffer();
  REQUIRE(buf.empty());

  float data[] = {0, 1, 0, -1};
  buf = audio::buffer(data, 1, audio::buffer_ordering::interleaved);
  REQUIRE(!buf.empty());
}

TEST_CASE( "Buffer size_bytes()", "[buffer]") {
  auto buf = audio::buffer();
  REQUIRE(buf.size_bytes() == 0);

  float data[] = {0, 1, 0, -1};
  buf = audio::buffer(data, 1, audio::buffer_ordering::interleaved);
  REQUIRE(buf.size_bytes() == 16);

  buf = audio::buffer(data, 4, audio::buffer_ordering::interleaved);
  REQUIRE(buf.size_bytes() == 16);
}

TEST_CASE( "Buffer raw data access", "[buffer]") {
  auto buf = audio::buffer();
  REQUIRE(buf.raw().empty());

  float data[] = {0, 1, 0, -1};
  buf = audio::buffer(data, 1, audio::buffer_ordering::interleaved);

  REQUIRE(!buf.raw().empty());
  REQUIRE(buf.raw().data() == data);
  REQUIRE(buf.raw().size() == sizeof(data) / sizeof(data[0]));
  REQUIRE(buf.get_ordering() == audio::buffer_ordering::interleaved);

  buf = audio::buffer(data, 1, audio::buffer_ordering::deinterleaved);
  REQUIRE(!buf.raw().empty());
  REQUIRE(buf.get_ordering() == audio::buffer_ordering::deinterleaved);
}




TEST_CASE( "Buffer channels size", "[buffer]") {
  auto buf = audio::buffer();
  REQUIRE(buf.channels().size() == 0);

  float data[] = {0, 1, 0, -1};
  buf = audio::buffer(data, 1, audio::buffer_ordering::interleaved);
  REQUIRE(buf.channels().size() == 1);

  buf = audio::buffer(data, 2, audio::buffer_ordering::interleaved);
  REQUIRE(buf.channels().size() == 2);
}

TEST_CASE( "Buffer channels loop", "[buffer]") {
  auto buf = audio::buffer();
  for (auto& ch : buf.channels())
    FAIL();
}

TEST_CASE( "Buffer channels begin and end", "[buffer]") {
  auto buf = audio::buffer();

  auto b = buf.channels().begin();
  auto e = buf.channels().end();
  REQUIRE(b == e);

  float data[] = {0, 1, 0, -1};
  buf = audio::buffer(data, 1, audio::buffer_ordering::interleaved);
  
  b = buf.channels().begin();
  e = buf.channels().end();
  REQUIRE(b != e);
}

TEST_CASE( "Buffer channels increment/decrement", "[buffer]") {
  float more_data[] = {0, 1, 0, -1, 0, 2, 0, -2};
  auto buf = audio::buffer(more_data, 4, audio::buffer_ordering::interleaved);
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
  auto buf = audio::buffer(more_data, 4, audio::buffer_ordering::interleaved);
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
  auto buf = audio::buffer(more_data, 4, audio::buffer_ordering::interleaved);
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

TEST_CASE( "Buffer channels empty()", "[buffer]") {
  auto buf = audio::buffer();
  REQUIRE(buf.channels().empty());

  float more_data[] = {0, 1, 0, -1, 0, 2, 0, -2};
  buf = audio::buffer(more_data, 4, audio::buffer_ordering::interleaved);
  REQUIRE(!buf.channels().empty());
}
