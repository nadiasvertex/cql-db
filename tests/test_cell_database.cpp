#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/database.h>

#include <gtest/gtest.h>

TEST(DbTest, CanCreate)
{
  std::unique_ptr<lattice::cell::database> db;
  ASSERT_NO_THROW(db =std::unique_ptr<lattice::cell::database>(
      new lattice::cell::database()
      )
      );
}

TEST(DbTest, CanCreateTable)
{
  lattice::cell::database db;

  EXPECT_TRUE(
    db.create_table("test1", {
        new lattice::cell::column { lattice::cell::column::data_type::integer, "id",  4 },
        new lattice::cell::column { lattice::cell::column::data_type::bigint, "c1", 8 },
    })
  );

}
