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
auto *c1 = new lattice::cell::column
  {
    .type = lattice::cell::column::data_type::integer,
    .name = "col1"
  }
;

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
}
;

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
}
;

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
}
;

t.set_column_definition(0, c1);

std::uint32_t value1 = 100, value2 = 0;

t.insert_row(1,
   {
   true
   },
   static_cast<std::uint8_t*>(
   static_cast<void*>(&value1)),
   sizeof(value1)
   );

  EXPECT_TRUE(t.fetch_row(1,
{
true
},
static_cast<std::uint8_t*>(
static_cast<void*>(&value2)),
sizeof(value2)
));
}

TEST(TableTest, CanFetchManyRows)
{
lattice::cell::table t
{
1
};

auto *c1 = new lattice::cell::column
{
.type = lattice::cell::column::data_type::integer,
.name = "col1"
}
;

t.set_column_definition(0, c1);

std::uint32_t value[10000], value2[10000];

for (auto i = 0; i < 10000; ++i)
    {
      value[i]=i;

      t.insert_row(i+1,
      {true},
      static_cast<std::uint8_t*>(
      static_cast<void*>(&value[i])),
      sizeof(std::uint32_t));

      EXPECT_TRUE(t.fetch_row(i+1,
{
true
},
static_cast<std::uint8_t*>(
static_cast<void*>(&value2[i])),
sizeof(std::uint32_t)
));

EXPECT_EQ(value[i], value2[i]);

}

  // Fetch again, just to make sure it's still right.
  for (auto i = 0; i < 10000; ++i)
    {
      value2[i] = 0;

      EXPECT_TRUE(t.fetch_row(i+1,
{
true
},
static_cast<std::uint8_t*>(
static_cast<void*>(&value2[i])),
sizeof(std::uint32_t)
));

EXPECT_EQ(value[i], value2[i]);

}

}

TEST(TableTest, CanConvertToBinary)
{

lattice::cell::table t
{
2
};

auto *c1 = new lattice::cell::column
{
.type = lattice::cell::column::data_type::integer,
.name = "col1"
}
;

auto *c2 = new lattice::cell::column
{
.type = lattice::cell::column::data_type::bigint,
.name = "col2"
}
;

t.set_column_definition(0, c1);
t.set_column_definition(1, c2);

lattice::cell::table::text_tuple_type text_data
{
"123", "1234567890"
};

uint8_t buffer[64];
ASSERT_TRUE(t.to_binary({true,true}, text_data, buffer, sizeof(buffer)));

}

TEST(TableTest, CanFetchManyRows2)
{
  lattice::cell::table t
  {
  2
  };

  auto *c1 = new lattice::cell::column
  {
  .type = lattice::cell::column::data_type::integer,
  .name = "col1"
  }
  ;

  auto *c2 = new lattice::cell::column
  {
  .type = lattice::cell::column::data_type::bigint,
  .name = "col2"
  }
  ;

  t.set_column_definition(0, c1);
  t.set_column_definition(1, c2);

  lattice::cell::table::text_tuple_type text_data
  {
  "123", "1234567890"
  };

  auto present = lattice::cell::table::column_present_type({true,true});

  // Create a binary buffer.
  uint8_t buffer[12], out_buffer[12];

  memset(buffer, 0, sizeof(buffer));
  memset(out_buffer, 0, sizeof(buffer));

  ASSERT_TRUE(t.to_binary(present, text_data, buffer, sizeof(buffer)));

  // Write this data into the table, then read it back.
  for (auto i = 0; i < 10000; ++i)
      {
        t.insert_row(i+1,
        present,
        buffer,
        sizeof(buffer));

        EXPECT_TRUE(t.fetch_row(i+1,
          present,
          out_buffer,
          sizeof(out_buffer)
        ));

        EXPECT_EQ(0, memcmp(buffer, out_buffer, sizeof(buffer)));
      }
}
