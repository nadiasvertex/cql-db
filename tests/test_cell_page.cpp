#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/page.h>

#include <gtest/gtest.h>

TEST(PageTest, CanCreate)
{
  std::unique_ptr<lattice::cell::page> page;
  ASSERT_NO_THROW(
      page = std::unique_ptr<lattice::cell::page>(new lattice::cell::page())
      );
}

TEST(PageTest, CanWrite)
{
  lattice::cell::page page;

  int a = 5;

  EXPECT_EQ(sizeof(a), page.insert_object(0, a));
}

TEST(PageTest, CanWriteString)
{
  lattice::cell::page page;

  std::string s("This is a test string.");

  EXPECT_EQ(s.size() + sizeof(std::uint32_t), page.insert_object(0, s));
}

TEST(PageTest, CanRead)
{
  lattice::cell::page page;

  int a = 5, b = 0;

  EXPECT_EQ(sizeof(a), page.insert_object(0, a));
  EXPECT_TRUE(std::get<0>(page.fetch_object(0, b)));
  EXPECT_EQ(a, b);
}

TEST(PageTest, CanReadString)
{
  lattice::cell::page page;

  std::string s("This is a test string.");

  EXPECT_EQ(s.size() + sizeof(std::uint32_t), page.insert_object(0, s));

  std::string n;

  EXPECT_TRUE(std::get<0>(page.fetch_object(0,n)));
  EXPECT_EQ(s, n);
}

TEST(PageTest, CanWriteMany)
{
  lattice::cell::page page;

  page.set_max_atom_size(1024);

  int a = 5;

  for (auto i = 0; i < 10000; ++i, ++a)
    {
      EXPECT_EQ(sizeof(a), page.insert_object(i, a));
    }
}

TEST(PageTest, CanReadMany)
{
  lattice::cell::page page;

  page.set_max_atom_size(1024);

  int a = 5, b = 0;

  for (auto i = 0; i < 10000; ++i, ++a)
    {
      EXPECT_EQ(sizeof(a), page.insert_object(i, a));
    }

  a = 5;

  for (auto i = 0; i < 10000; ++i, ++a)
    {
      b = 0;
      EXPECT_TRUE(std::get<0>(page.fetch_object(i, b)));
      EXPECT_EQ(a, b);
    }
}

TEST(PageTest, CanDelete)
{
  lattice::cell::page page;

  int a = 5;

  page.insert_object(0, a);
  page.delete_object(0);

  EXPECT_FALSE(std::get<0>(page.fetch_object(0, a)));
}

TEST(PageTest, CanDeleteMany)
{
  lattice::cell::page page;

  page.set_max_atom_size(1024);

  int a = 5;

  for (auto i = 0; i < 10000; ++i, ++a)
    {
      page.insert_object(i, a);
    }

  for (auto i = 0; i < 10000; ++i)
    {
      page.delete_object(i);
      EXPECT_FALSE(std::get<0>(page.fetch_object(i, a)));
    }
}

