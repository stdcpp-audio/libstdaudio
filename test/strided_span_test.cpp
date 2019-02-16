// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#include <audio>
#include "catch/catch.hpp"

using namespace std::experimental::audio;

TEST_CASE( "Empty span", "[strided_span]") {
  strided_span<int> ss;

  REQUIRE(ss.data() == nullptr);
  REQUIRE(ss.size() == 0);
  REQUIRE(ss.empty());
}

TEST_CASE( "Empty span with zero stride", "[strided_span]") {
  strided_span<int> ss{nullptr, 0, 0};

  REQUIRE(ss.data() == nullptr);
  REQUIRE(ss.size() == 0);
  REQUIRE(ss.empty());
}

TEST_CASE( "Span with unity stride", "[strided_span]") {
  std::array<int, 3> a{11, 21, 31};
  strided_span ss(a.data(), a.size(), 1);

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
  strided_span ss(a.data(), a.size(), 3);

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

TEST_CASE( "at", "[strided_span]") {
  std::array<int, 9> a{1, 2, 3, 4, 5, 6, 7, 8, 9};
  strided_span sspan(a.data(), a.size(), 3);

  REQUIRE_THROWS_AS(sspan.at(-1), std::out_of_range);
  REQUIRE(sspan.at(0) == sspan[0]);
  REQUIRE(sspan.at(1) == sspan[1]);
  REQUIRE(sspan.at(2) == sspan[2]);
  REQUIRE_THROWS_AS(sspan.at(3), std::out_of_range);
}

TEST_CASE( "Unequal size", "[strided_span]" ) {
  std::array<int, 8> a{1, 2, 3, 4, 5, 6, 7, 8};
  strided_span sspan1(a.data(), a.size(), 2);
  strided_span sspan2(a.data(), a.size() - 2, 2);

  REQUIRE_FALSE(sspan1 == sspan2);
  REQUIRE(sspan1 != sspan2);
}

TEST_CASE( "Unequal stride", "[strided_span]" ) {
  std::array<int, 8> a{1, 2, 3, 4, 5, 6, 7, 8};
  strided_span sspan1(a.data(), a.size(), 2);
  strided_span sspan2(a.data(), a.size(), 4);

  REQUIRE_FALSE(sspan1 == sspan2);
  REQUIRE(sspan1 != sspan2);
}

TEST_CASE( "Equal content, unequal data pointer", "[strided_span]" ) {
  std::array<int, 8> a1{1, 2, 3, 4, 5, 6, 7, 8};
  std::array<int, 8> a2{1, 2, 3, 4, 5, 6, 7, 8};
  REQUIRE(a1.data() != a2.data());

  strided_span sspan1(a1.data(), a1.size(), 2);
  strided_span sspan2(a2.data(), a2.size(), 2);

  REQUIRE(sspan1 == sspan2);
  REQUIRE_FALSE(sspan1 != sspan2);
}