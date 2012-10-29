#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/table.h>

#include <gtest/gtest.h>

TEST(TableTest, CanCreate)
{
  std::unique_ptr<lattice::cell::table<2>> t;
    ASSERT_NO_THROW(t =std::unique_ptr<lattice::cell::table<2>>(
           new lattice::cell::table<2>()
          )
    );
}

TEST(TableTest, CanSetColumnDefinition)
{
  lattice::cell::table<1> t;
  lattice::cell::column c1 {
    .type = lattice::cell::column::data_type::integer,
    .name = "col1"
  };

  EXPECT_TRUE(t.set_column_definition(0, c1));
}
