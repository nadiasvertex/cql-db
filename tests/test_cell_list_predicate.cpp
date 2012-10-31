#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/list_predicate.h>
#include <cell/cpp/page_cursor.h>

#include <gtest/gtest.h>

TEST(ListPredicateTest, CanCreate)
{
  using namespace lattice::cell;

 std::unique_ptr<list_predicate> sp;

  ASSERT_NO_THROW(
      sp = std::unique_ptr<list_predicate>(
          new list_predicate())
      );
}

TEST(ListPredicateTest, ContainsWorks)
{
  auto def = new lattice::cell::column {
		  lattice::cell::column::data_type::integer,
		  "test_col", 4, 0, false
  };

  lattice::cell::page page(def);

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

