#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/table.h>

#include <gtest/gtest.h>

TEST(TableTest, CanCreate)
{
   std::unique_ptr<lattice::cell::table> t;
   ASSERT_NO_THROW(
         t =std::unique_ptr<lattice::cell::table>( new lattice::cell::table(0, 2) ));
}

TEST(TableTest, CanGetColumnCount)
{
   lattice::cell::table t
      {
      0, 1
      };

   EXPECT_EQ(1, t.get_number_of_columns());
}

TEST(TableTest, CanSetColumnDefinition)
{
   lattice::cell::table t
      {
      0, 1
      };

   auto *c1 = new lattice::cell::column
      {
      lattice::cell::column::data_type::integer, "col1"
      };

   EXPECT_TRUE(t.set_column_definition(0, c1));
}

TEST(TableTest, CanInsertRow)
{
   using namespace lattice::cell;

   lattice::cell::table t
      {
      0, 1
      };
   auto *c1 = new column
      {
      column::data_type::integer, "col1"
      };

   t.set_column_definition(0, c1);

   std::uint32_t value = 100;
   table::row_type row;

   ASSERT_TRUE(
         t.insert_row(row, {true}, static_cast<std::uint8_t*>( static_cast<void*>(&value)), sizeof(value) ));
}

TEST(TableTest, CanInsertManyRows)
{
   using namespace lattice::cell;

   table t
      {
      0, 1
      };
   auto *c1 = new column
      {
      column::data_type::integer, "col1"
      };

   t.set_column_definition(0, c1);

   std::uint32_t value[10000];

   for (auto i = 0; i < 10000; ++i)
      {
         table::row_type row;
         value[i] = i;
         ASSERT_TRUE(
               t.insert_row(row, {true}, static_cast<std::uint8_t*>( static_cast<void*>(&value[i])), sizeof(std::uint32_t) ));
      }
}

TEST(TableTest, CanFetchRow)
{
   using namespace lattice::cell;

   table t
      {
      0, 1
      };
   auto *c1 = new column
      {
      column::data_type::integer, "col1"
      };

   t.set_column_definition(0, c1);

   std::uint32_t value1 = 100, value2 = 0;

   table::row_type row;

   t.insert_row(row,
      {
      true
      }, static_cast<std::uint8_t*>(static_cast<void*>(&value1)),
         sizeof(value1));

   t.commit_row(1, row);

   EXPECT_TRUE(
         t.fetch_row(1, { true }, static_cast<std::uint8_t*>( static_cast<void*>(&value2)), sizeof(value2) ));
}

TEST(TableTest, CanFetchManyRows)
{
   using namespace lattice::cell;

   table t
      {
      0, 1
      };

   auto *c1 = new column
      {
      column::data_type::integer, "col1"
      };

   t.set_column_definition(0, c1);

   std::uint32_t value[10000], value2[10000];

   for (auto i = 0; i < 10000; ++i)
      {
         table::row_type row;
         value[i] = i;

         t.insert_row(row,
            {
            true
            }, static_cast<std::uint8_t*>(static_cast<void*>(&value[i])),
               sizeof(std::uint32_t));

         t.commit_row(i + 1, row);

         EXPECT_TRUE(
               t.fetch_row(i+1, { true }, static_cast<std::uint8_t*>( static_cast<void*>(&value2[i])), sizeof(std::uint32_t) ));

         EXPECT_EQ(value[i], value2[i]);

      }

   // Fetch again, just to make sure it's still right.
   for (auto i = 0; i < 10000; ++i)
      {
         value2[i] = 0;

         EXPECT_TRUE(
               t.fetch_row(i+1, { true }, static_cast<std::uint8_t*>( static_cast<void*>(&value2[i])), sizeof(std::uint32_t) ));

         EXPECT_EQ(value[i], value2[i]);

      }

}

TEST(TableTest, CanConvertToBinary)
{
   using namespace lattice::cell;

   table t
      {
      0, 2
      };

   auto *c1 = new column
      {
      column::data_type::integer, "col1"
      };

   auto *c2 = new column
      {
      column::data_type::bigint, "col2"
      };

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
   using namespace lattice::cell;

   lattice::cell::table t
      {
      0,2
      };

   auto *c1 = new column
      {
      column::data_type::integer, "col1"
      };

   auto *c2 = new column
      {
      column::data_type::bigint, "col2"
      };

   t.set_column_definition(0, c1);
   t.set_column_definition(1, c2);

   lattice::cell::table::text_tuple_type text_data
      {
      "123", "1234567890"
      };

   auto present = lattice::cell::table::column_present_type(
      {
      true, true
      });

   // Create a binary buffer.
   uint8_t buffer[12], out_buffer[12];

   memset(buffer, 0, sizeof(buffer));
   memset(out_buffer, 0, sizeof(buffer));

   ASSERT_TRUE(t.to_binary(present, text_data, buffer, sizeof(buffer)));

   // Write this data into the table, then read it back.
   for (auto i = 0; i < 10000; ++i)
      {
         table::row_type row;
         t.insert_row(row, present, buffer, sizeof(buffer));
         t.commit_row(i+1, row);

         EXPECT_TRUE(
               t.fetch_row(i+1, present, out_buffer, sizeof(out_buffer) ));

         EXPECT_EQ(0, memcmp(buffer, out_buffer, sizeof(buffer)));
      }
}
