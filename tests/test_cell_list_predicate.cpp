#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/list_predicate.h>

#include <gtest/gtest.h>

TEST(ListPredicateTest, CanCreate)
{
 std::unique_ptr<lattice::cell::list_predicate> sp;

  ASSERT_NO_THROW(
      sp = std::unique_ptr<lattice::cell::list_predicate>(
          new lattice::cell::list_predicate())
      );
}


TEST(ListPredicateTest, CanCompare)
{
  lattice::cell::page page;

  int a = 5;

  for (auto i = 0; i < 10000; ++i, ++a)
    {
      EXPECT_EQ(sizeof(a), page.insert_object(i, a));
    }

  lattice::cell::page_cursor cursor(page);
  lattice::cell::list_predicate pred;

  for (auto i=100; i<200; ++i)
    {
      pred.add_value(lattice::cell::column::data_type::integer, i);
    }

  int b = 5;
  while(!cursor.end_of_page()) 
   {

    if (b>=100 && b<200)
      {
        EXPECT_TRUE(pred.contains(cursor));
      }
    else
      {
        EXPECT_FALSE(pred.contains(cursor));
      }

     // Go to the next record.
     cursor.advance();
     ++b;
   }
}

