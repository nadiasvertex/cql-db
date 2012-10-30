#include <cstdint>
#include <sstream>

#include <cell/cpp/unstringify.h>
#include <cell/cpp/compare.h>

#include <gtest/gtest.h>

TEST(CompareTest, Int)
{
  std::int32_t a=100,b=200;

  EXPECT_EQ(0, lattice::cell::cmp(a, &a));
  EXPECT_EQ(1, lattice::cell::cmp(b, &a));
  EXPECT_EQ(-1, lattice::cell::cmp(a, &b));
}

TEST(CompareTest, String1)
{
  std::string t1("a test string");
  std::string t2("a test string longer");
  std::string t3("different test string");

  uint8_t buffer[1024];
  std::size_t offsets[2];

  offsets[0] = lattice::cell::unstringify::to_binary(t1, buffer);
  offsets[1] = lattice::cell::unstringify::to_binary(t2, buffer+offsets[0]) + offsets[0];
               lattice::cell::unstringify::to_binary(t3, buffer+offsets[1]);

  std::stringstream ss;

  ss.write(static_cast<char *>(
      static_cast<void*>(buffer)
  ), t1.size() + t2.size() + t3.size() + (4*3));

  ss.seekg(0, std::ios::beg);
  EXPECT_EQ(0, lattice::cell::cmp(t1, ss));

  ss.seekg(offsets[0], std::ios::beg);
  EXPECT_EQ(-1, lattice::cell::cmp(t1, ss));

  ss.seekg(offsets[1], std::ios::beg);
  EXPECT_EQ(-1, lattice::cell::cmp(t1, ss));

  ss.seekg(0, std::ios::beg);
  EXPECT_EQ(1, lattice::cell::cmp(t2, ss));

  ss.seekg(offsets[0], std::ios::beg);
  EXPECT_EQ(0, lattice::cell::cmp(t2, ss));

  ss.seekg(offsets[1], std::ios::beg);
  EXPECT_EQ(-1, lattice::cell::cmp(t2, ss));

  ss.seekg(0, std::ios::beg);
  EXPECT_EQ(1, lattice::cell::cmp(t3, ss));

  ss.seekg(offsets[0], std::ios::beg);
  EXPECT_EQ(1, lattice::cell::cmp(t3, ss));

  ss.seekg(offsets[1], std::ios::beg);
  EXPECT_EQ(0, lattice::cell::cmp(t3, ss));
}

TEST(CompareTest, String2)
{
  std::string t1("a test string");
  std::string t2("a test string longer");
  std::string t3("different test string");

  uint8_t buffer[1024];
  std::size_t offsets[2];

  offsets[0] = lattice::cell::unstringify::to_binary(t1, buffer);
  offsets[1] = lattice::cell::unstringify::to_binary(t2, buffer+offsets[0]) + offsets[0];
               lattice::cell::unstringify::to_binary(t3, buffer+offsets[1]);


  EXPECT_EQ(0, lattice::cell::cmp(t1, buffer));
  EXPECT_EQ(-1, lattice::cell::cmp(t1, buffer+offsets[0]));
  EXPECT_EQ(-1, lattice::cell::cmp(t1, buffer+offsets[1]));


  EXPECT_EQ(1, lattice::cell::cmp(t2, buffer));
  EXPECT_EQ(0, lattice::cell::cmp(t2, buffer+offsets[0]));
  EXPECT_EQ(-1, lattice::cell::cmp(t2, buffer+offsets[1]));


  EXPECT_EQ(1, lattice::cell::cmp(t3, buffer));
  EXPECT_EQ(1, lattice::cell::cmp(t3, buffer+offsets[0]));
  EXPECT_EQ(0, lattice::cell::cmp(t3, buffer+offsets[1]));
}

