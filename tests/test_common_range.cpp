#include <memory>
#include <common/cpp/range.h>

#include <gtest/gtest.h>

TEST(RangeTest, CanCreate)
{
   using namespace lattice::common;

   std::unique_ptr<range<int>> r;
   ASSERT_NO_THROW( r = std::unique_ptr<range<int>>(new range<int>()));
}

TEST(RangeTest, CanInsertPoint)
{
   using namespace lattice::common;

   range<int> r;

   ASSERT_NO_THROW(r.insert(10));
}

TEST(RangeTest, ContainsOnePoint)
{
   using namespace lattice::common;

   range<int> r;

   r.insert(10);
   ASSERT_TRUE(r.contains(10));
}

TEST(RangeTest, ContainsManyPoints)
{
   using namespace lattice::common;

   range<int> r;

   for (auto i = 50; i < 70; ++i)
      {
         r.insert(i);
      }

   for (auto i = 50; i < 70; ++i)
      {
         ASSERT_TRUE(r.contains(i));
      }

   // There should only be one segment.
   ASSERT_EQ(1, r.size());
}

TEST(RangeTest, ContainsManySegments)
{
   using namespace lattice::common;

   range<int> r;

   range<int>::size_type num_segs = 0;
   for (auto i = 50; i < 500; i += 10)
      {
         num_segs++;
         for (auto j = i + 2; j < i + 9; ++j)
            {
               r.insert(j);
            }
      }

   for (auto i = 50; i < 500; i += 10)
      {
         for (auto j = i + 2; j < i + 9; ++j)
            {
               ASSERT_TRUE(r.contains(j));
            }

         ASSERT_FALSE(r.contains(i));
         ASSERT_FALSE(r.contains(i+1));
         ASSERT_FALSE(r.contains(i+9));
      }

   ASSERT_EQ(num_segs, r.size());
}
