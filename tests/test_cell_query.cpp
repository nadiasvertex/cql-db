#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/query.h>

#include <gtest/gtest.h>

class QueryTest: public ::testing::Test
{
public:
  lattice::cell::database *db;

  virtual void SetUp()
  {
    using namespace lattice::cell;

    db = new database();
    db->create_table("test_table_1",
          {
          new column
                {
                column::data_type::integer, "id", 4
                },
              new column
                {
                column::data_type::bigint, "c1", 8
                },
              new column
                {
                column::data_type::varchar, "c2", 0
                }
          });
  }

  virtual void TearDown()
  {
    delete db;
  }

};

TEST_F(QueryTest, CanCreate)
{
  using namespace lattice::cell;

  std::unique_ptr<query> q;
  ASSERT_NO_THROW(q =std::unique_ptr<query>(
      new query(*db, "")
      )
      );
}

TEST_F(QueryTest, CanSelectOne)
{
  using namespace lattice::cell;

  query q(*db, "select 1");

  auto r = q.fetch_one();

  ASSERT_EQ(1, r.size());
  EXPECT_EQ(std::string("1"), r[0]);
}

TEST_F(QueryTest, CanSelectOnePlusOne)
{
  using namespace lattice::cell;

  query q(*db, "select 1+1");

  auto r = q.fetch_one();

  ASSERT_EQ(1, r.size());
  EXPECT_EQ(std::string("2"), r[0]);
}

TEST_F(QueryTest, CanSelectComplexAddition)
{
  using namespace lattice::cell;

  query q(*db, "select 1+2+3+4+5+6+7+8+9+10");

  auto r = q.fetch_one();

  ASSERT_EQ(1, r.size());
  EXPECT_EQ(std::string("55"), r[0]);
}

TEST_F(QueryTest, CanSelectMultiColumnAddition)
{
  using namespace lattice::cell;

  query q(*db, "select 1+2+3+4+5+6+7+8+9+10, 20+30+40+50");

  auto r = q.fetch_one();

  ASSERT_EQ(2, r.size());
  EXPECT_EQ(std::string("55"), r[0]);
  EXPECT_EQ(std::string("140"), r[1]);
}

TEST_F(QueryTest, CanSelectTenTimesTen)
{
  using namespace lattice::cell;

  query q(*db, "select 10*10");

  auto r = q.fetch_one();

  ASSERT_EQ(1, r.size());
  EXPECT_EQ(std::string("100"), r[0]);
}

TEST_F(QueryTest, CanSelectMultiColumnMultiplication)
{
  using namespace lattice::cell;

  query q(*db, "select 10*10*10, 2*4*8");

  auto r = q.fetch_one();

  ASSERT_EQ(2, r.size());
  EXPECT_EQ(std::string("1000"), r[0]);
  EXPECT_EQ(std::string("64"), r[1]);
}

TEST_F(QueryTest, CanSelectMultiColumnDivision)
{
  using namespace lattice::cell;

  query q(*db, "select 100/10, 16/4/2");

  auto r = q.fetch_one();

  ASSERT_EQ(2, r.size());
  EXPECT_EQ(std::string("10"), r[0]);
  EXPECT_EQ(std::string("2"), r[1]);
}

TEST_F(QueryTest, CanSelectMultiColumnSubtraction)
{
  using namespace lattice::cell;

  query q(*db, "select 25-5, 17-7");

  auto r = q.fetch_one();

  ASSERT_EQ(2, r.size());
  EXPECT_EQ(std::string("20"), r[0]);
  EXPECT_EQ(std::string("10"), r[1]);
}


