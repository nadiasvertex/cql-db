#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/predicate_solver.h>

#include <gtest/gtest.h>

TEST(ScalarPredicateTest, CanCreate)
{
 std::unique_ptr<lattice::cell::scalar_predicate> sp;

  ASSERT_NO_THROW(
      sp = std::unique_ptr<lattice::cell::scalar_predicate>(
          new lattice::cell::scalar_predicate())
      );
}


TEST(ScalarPredicateTest, CanCompare)
{
  lattice::cell::page page;

  int a = 5;

  for (auto i = 0; i < 10000; ++i, ++a)
    {
      EXPECT_EQ(sizeof(a), page.insert_object(i, a));
    }

  lattice::cell::page_cursor cursor(page);
  lattice::cell::scalar_predicate pred;

  pred.set_value(lattice::cell::column::data_type::integer, 100);

  int b = 5;
  while(!cursor.end_of_page()) 
   {

    if (b==100) 
      {
        EXPECT_EQ(0, pred.cmp(cursor));
      }
    else if (b<100)
      {
        EXPECT_EQ(1, pred.cmp(cursor));
      }
    else
      {
        EXPECT_EQ(-1, pred.cmp(cursor));
      }

     // Go to the next record.
     cursor.advance();
     ++b;
   }
}

