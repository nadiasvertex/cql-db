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

   table::text_tuple_type text_data
      {
      "123", "1234567890"
      };

   std::string buffer;
   ASSERT_TRUE(t.to_binary({true,true}, text_data, buffer));
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

   table::text_tuple_type text_data
      {
      "100"
      };

   std::string buffer;
   t.to_binary(
      {
      true
      }, text_data, buffer);

   row_id rid;
   transaction_id tid;

   ASSERT_EQ(table::insert_code::SUCCESS,
         t.insert_row(tid, rid, {true}, buffer));
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
   transaction_id tid;

   for (auto i = 0; i < 10000; ++i)
      {
         row_id rid;
         table::text_tuple_type text_data
            {
            std::to_string(i)
            };

         std::string buffer;
         t.to_binary(
            {
            true
            }, text_data, buffer);

         ASSERT_EQ(table::insert_code::SUCCESS,
               t.insert_row(tid, rid, {true}, buffer ));
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

   transaction_id tid;
   row_id rid;

   std::string in_buffer;
   std::stringstream out_buffer;

   table::text_tuple_type text_data
      {
      "100"
      };

   t.to_binary(
      {
      true
      }, text_data, in_buffer);

   t.insert_row(tid, rid,
      {
      true
      }, in_buffer);

   t.commit_row(tid, rid);

   ASSERT_EQ(table::fetch_code::SUCCESS,
         t.fetch_row(tid, rid, { true }, out_buffer ));

   ASSERT_EQ(in_buffer, out_buffer.str());
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

   transaction_id tid;

   for (auto i = 0; i < 10000; ++i)
      {
         row_id rid;
         table::text_tuple_type text_data
            {
            std::to_string(i)
            };

         std::string in_buffer;
         t.to_binary(
            {
            true
            }, text_data, in_buffer);

         t.insert_row(tid, rid,
            {
            true
            }, in_buffer);

         t.commit_row(tid, rid);

         std::stringstream out_buffer;

         EXPECT_EQ(table::fetch_code::SUCCESS, t.fetch_row(tid, rid, { true }, out_buffer));
         EXPECT_EQ(in_buffer, out_buffer.str());

      }
}
