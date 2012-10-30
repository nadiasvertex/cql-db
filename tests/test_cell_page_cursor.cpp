#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/page_cursor.h>

#include <gtest/gtest.h>

TEST(PageCursorTest, CanCreate)
{
  lattice::cell::page page;
  std::unique_ptr<lattice::cell::page_cursor> page_cursor;

  ASSERT_NO_THROW(
      page_cursor = std::unique_ptr<lattice::cell::page_cursor>(
          new lattice::cell::page_cursor(page))
      );
}

TEST(PageCursorTest, CanIterate)
{
  lattice::cell::page page;

  int a = 5;

  for (auto i = 0; i < 10000; ++i, ++a)
    {
      EXPECT_EQ(sizeof(a), page.insert_object(i, a));
    }

  lattice::cell::page_cursor cursor(page);

  int i=0;
  while(!cursor.end_of_page()) {
      int b = 0;

      EXPECT_EQ(sizeof(b), cursor.value(b));
      EXPECT_EQ(i+5, b);
      EXPECT_EQ(i, cursor.oid());

      // Go to the next record.
      cursor.advance();
      ++i;
  }
}

