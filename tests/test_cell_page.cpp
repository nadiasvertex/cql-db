#include <memory>
#include <sstream>

#include <cell/cpp/page.h>

#include <gtest/gtest.h>

TEST(PageTest, CanCreate) {
  std::unique_ptr<lattice::cell::page> page;
  ASSERT_NO_THROW(page = std::unique_ptr<lattice::cell::page>(new lattice::cell::page()));
}

TEST(PageTest, CanWrite) {
  lattice::cell::page page;

  int a = 5;

  EXPECT_EQ(sizeof(a), page.insert_row(0, a));
}
