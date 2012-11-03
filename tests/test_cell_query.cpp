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

TEST_F(QueryTest, CanSelect)
{
  using namespace lattice::cell;

  query q(*db, "select 1");

  auto r = q.fetch_one();

  EXPECT_EQ(1, r.size());
}

