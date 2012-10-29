#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/table.h>

#include <gtest/gtest.h>

TEST(TableTest, CanCreate)
{
  std::unique_ptr<lattice::cell::table> t;
  ASSERT_NO_THROW(t =std::unique_ptr<lattice::cell::table>(
      new lattice::cell::table(2)
      )
      );
}

TEST(TableTest, CanGetColumnCount)
{
  lattice::cell::table t
    {
    1
    };

  EXPECT_EQ(1, t.get_number_of_columns());
}

TEST(TableTest, CanSetColumnDefinition)
{
  lattice::cell::table t
    {
    1
    };
  auto *c1 = new lattice::cell::column {
    .type = lattice::cell::column::data_type::integer,
    .name = "col1"
  };

    EXPECT_TRUE(t.set_column_definition(0, c1));
}

TEST(TableTest, CanInsertRow)
{
  lattice::cell::table t
    {
    1
    };
  auto *c1 = new lattice::cell::column
    {
      .type = lattice::cell::column::data_type::integer,
      .name = "col1"
    };

  t.set_column_definition(0, c1);

  std::uint32_t value = 100;

  ASSERT_TRUE(
      t.insert_row(1, {true},
      static_cast<std::uint8_t*>(
      static_cast<void*>(&value)),
      sizeof(value)
      ));
}

TEST(TableTest, CanInsertManyRows)
{
  lattice::cell::table t
    {
    1
    };
  auto *c1 = new lattice::cell::column
    {
      .type = lattice::cell::column::data_type::integer,
      .name = "col1"
    };

  t.set_column_definition(0, c1);

  std::uint32_t value[10000];

  for (auto i = 0; i < 10000; ++i)
    {
      value[i]=i;
      ASSERT_TRUE(
          t.insert_row(i+1, {true},
          static_cast<std::uint8_t*>(
          static_cast<void*>(&value[i])),
          sizeof(std::uint32_t)
          ));
    }
}

TEST(TableTest, CanFetchRow)
{
  lattice::cell::table t
    {
    1
    };
  auto *c1 = new lattice::cell::column
    {
      .type = lattice::cell::column::data_type::integer,
      .name = "col1"
    };

  t.set_column_definition(0, c1);

  std::uint32_t value1 = 100, value2 = 0;

  /*t.insert_row(1,
   {
   true
   },
   static_cast<std::uint8_t*>(
   static_cast<void*>(&value1)),
   sizeof(value1)
   );*/

  /*EXPECT_TRUE(t.fetch_row(1,
   {
   true
   },
   static_cast<std::uint8_t*>(
   static_cast<void*>(&value2)),
   sizeof(value2)
   ));*/
}
