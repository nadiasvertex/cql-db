#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/table.h>
#include <cell/cpp/data_value.h>

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

         ASSERT_EQ(table::fetch_code::SUCCESS,
               t.fetch_row(tid, rid, { true }, out_buffer));
         EXPECT_EQ(in_buffer, out_buffer.str());

      }
}

TEST(TableTest, CanIsolateInsert)
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

   transaction_id tid_generator;
   transaction_id write_tid = tid_generator.next();
   transaction_id read_tid = tid_generator.next();

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

         t.insert_row(write_tid, rid,
            {
            true
            }, in_buffer);

         std::stringstream out_buffer;

         // Row has not been committed yet, so it shouldn't be visible.
         ASSERT_EQ(table::fetch_code::ISOLATED,
               t.fetch_row(read_tid, rid, { true }, out_buffer));

         // Commit row
         t.commit_row(write_tid, rid);

         // Row has been committed, so it should be visible.
         ASSERT_EQ(table::fetch_code::SUCCESS,
               t.fetch_row(read_tid, rid, { true }, out_buffer));
         EXPECT_EQ(in_buffer, out_buffer.str());
      }
}

TEST(TableTest, CanDisableIsolation)
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

   transaction_id tid_generator;
   transaction_id write_tid = tid_generator.next();
   transaction_id read_tid = tid_generator.next();

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

         t.insert_row(write_tid, rid,
            {
            true
            }, in_buffer);

         std::stringstream out_buffer;

         // Row has not been committed yet, but it should be visible.
         ASSERT_EQ(table::fetch_code::SUCCESS,
               t.fetch_row(read_tid, rid, { true }, out_buffer, isolation_level::READ_UNCOMMITTED));
      }
}

TEST(TableTest, CanSerializeTransactions)
{
   using namespace lattice::cell;
   using namespace std::rel_ops;

   table t
      {
      0, 1
      };

   auto *c1 = new column
      {
      column::data_type::integer, "col1"
      };

   t.set_column_definition(0, c1);

   ssi_lock_manager lm;

   t.set_ssi_lock_manager(&lm);

   transaction_id tid_generator;

   std::vector<row_id> rows;

   for (auto i = 0; i < 10000; ++i)
      {
         transaction_id tid1 = tid_generator.next();
         transaction_id tid2 = tid_generator.next();
         transaction_id tid3 = tid_generator.next();

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

         t.insert_row(tid1, rid,
            {
            true
            }, in_buffer);

         // Commit row
         t.commit_row(tid1, rid);

         // Save rid for later reading.
         rows.push_back(rid);
      }

   // Perform a multi-transaction read/read/write/write. Normally
   // this would cause write-skew, but we will request SERIALIZABLE
   // isolation.

   for (auto i = 0; i < 10000; ++i)
      {
         std::stringstream b1;
         std::stringstream b2;

         transaction_id tid2 = tid_generator.next();
         transaction_id tid3 = tid_generator.next();

         t.fetch_row(tid2, rows[i],
            {
            true
            }, b1, isolation_level::SERIALIZABLE);

         t.fetch_row(tid3, rows[i],
            {
            true
            }, b2, isolation_level::SERIALIZABLE);

         data_value dv(column::data_type::integer);

         b1.seekg(0);
         b1.seekp(0);
         // Increment b1 by one.
         dv.read(b1);
         dv.set_value(column::data_type::integer, dv.raw_int32_value() + 1);
         dv.write(b1);

         b2.seekg(0);
         b2.seekp(0);
         // Increment b2 by one.
         dv.read(b2);
         dv.set_value(column::data_type::integer, dv.raw_int32_value() + 1);
         dv.write(b2);

         // We are simulating this kind of traffic:
         //
         // T1: X = SELECT col1 FROM tbl;
         // T2: Y = SELECT col1 FROM tbl;
         // T1: UPDATE tbl SET col1 = X;
         // T1: COMMIT
         // T2: UPDATE tbl SET col1 = Y; -- T1's write gets lost.
         // T2: COMMIT
         //

         // T2 COMMIT

         row_id new_rid;
         ASSERT_EQ(table::update_code::SUCCESS,
               t.update_row(tid2, rows[i], { true }, b1.str(), new_rid, isolation_level::SERIALIZABLE));

         // Before the commit we make sure that we have no conflicts.
         auto results = lm.check_for_conflicts(tid2);
         ASSERT_FALSE(std::get<1>(results));

         t.commit_row(tid2, rows[i]);
         t.commit_row(tid2, new_rid);
         lm.commit(tid2);

         // T3 COMMIT

         ASSERT_EQ(table::update_code::SUCCESS,
               t.update_row(tid3, rows[i], { true }, b2.str(), new_rid, isolation_level::SERIALIZABLE));

         // Before the commit we make sure that we have no conflicts.
         results = lm.check_for_conflicts(tid3);
         ASSERT_TRUE(std::get<1>(results));
         ASSERT_FALSE(tid2 == std::get<0>(results));

         // Backout the transaction.
         lm.abort(std::get<0>(results));

      }

}
