// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental;

TEST_CASE( "Empty span", "[strided_span]") {
  audio::strided_span<int> ss;

  REQUIRE(ss.data() == nullptr);
  REQUIRE(ss.size() == 0);
  REQUIRE(ss.empty());
}

TEST_CASE( "Span without stride", "[strided_span]") {
  std::array<int, 3> a{11, 21, 31};
  audio::strided_span ss(a.data(), a.size(), 1);

  REQUIRE(ss.data() == a.data());
  REQUIRE(ss.size() == 3);
  REQUIRE(!ss.empty());
  REQUIRE(ss.stride() == 1);

  for (auto i = 0; i < ss.size(); ++i)
    REQUIRE(ss[i] == (10 * i) + 11);

  for (auto& item : ss)
    REQUIRE(item % 10 == 1);

  for (auto iter = ss.begin(); iter != ss.end(); ++iter)
    REQUIRE(*iter % 10 == 1);
}

TEST_CASE( "Span with stride", "[strided_span]") {
  std::array<int, 9> a{1, 2, 3, 4, 5, 6, 7, 8, 9};
  audio::strided_span ss(a.data(), a.size(), 3);

  REQUIRE(ss.data() == a.data());
  REQUIRE(ss.size() == 3);
  REQUIRE(!ss.empty());
  REQUIRE(ss.stride() == 3);

  for (auto i = 0; i < ss.size(); ++i)
    REQUIRE(ss[i] == (3 * i) + 1);

  for (auto& item : ss)
    REQUIRE(item % 3 == 1);

  for (auto iter = ss.begin(); iter != ss.end(); ++iter)
    REQUIRE(*iter % 3 == 1);
}

