#include <memory>
#include <sstream>

#include <cell/cpp/command_processor.h>

#include <gtest/gtest.h>

TEST(CellCmdProcessorTest, CanCreate)
{
   using namespace lattice::cell;

   std::unique_ptr<command_processor> cp;

   ASSERT_NO_THROW(
         cp = std::unique_ptr<command_processor>(new command_processor()));
}

TEST(CellCmdProcessorTest, CanCreateTable)
{
   using namespace lattice::cell;

   command_processor cp;

   EXPECT_TRUE(
         cp.create_table("test_table_1", { new lattice::cell::column { lattice::cell::column::data_type::integer, "id", 4 }, new lattice::cell::column { lattice::cell::column::data_type::bigint, "c1", 8 }, }));
}

TEST(CellCmdProcessorTest, CanExecutePrepare)
{
   using namespace lattice::cell;

   command_processor cp;

   cp.create_table("test_table_1",
      {
      new lattice::cell::column
         {
         lattice::cell::column::data_type::integer, "id", 4
         }, new lattice::cell::column
         {
         lattice::cell::column::data_type::bigint, "c1", 8
         },
      });

   CommandRequest request;

   request.set_kind(CommandRequest::PREPARE);

   auto* msg = request.mutable_prepare();

   msg->set_create_transaction(true);
   msg->add_cursors("test_table_1");

   auto resp = cp.process(request);

   ASSERT_TRUE(resp.has_kind());
   ASSERT_EQ(CommandResponse::PREPARE, resp.kind());

   auto& prepare_msg = resp.prepare();

   ASSERT_TRUE(prepare_msg.has_transaction_id());
   EXPECT_EQ(1, prepare_msg.cursor_ids_size());
}

TEST(CellCmdProcessorTest, CanExecuteInsert)
{
   using namespace lattice::cell;

   command_processor cp;

   cp.create_table("test_table_1",
      {
      new lattice::cell::column
         {
         lattice::cell::column::data_type::integer, "id", 4
         }, new lattice::cell::column
         {
         lattice::cell::column::data_type::bigint, "c1", 8
         },
      });

   CommandRequest request;

   request.set_kind(CommandRequest::PREPARE);

   auto* msg = request.mutable_prepare();
   msg->set_create_transaction(true);

   // Prepare for insert
   auto resp = cp.process(request);
   auto txn_id = resp.prepare().transaction_id();
   auto tbl_id = cp.get_database().get_table_id("test_table_1");
   auto t = cp.get_database().get_table(tbl_id);

   table::text_tuple_type text_data
      {
      "123", "1234567890"
      };

   std::string buffer;
   t->to_binary(
      {
      true, true
      }, text_data, buffer);

   // Create insert message
   CommandRequest request2;

   request2.set_kind(CommandRequest::INSERT);
   auto* msg2 = request2.mutable_insert();

   msg2->set_transaction_id(txn_id);
   msg2->set_table_id(tbl_id);
   msg2->set_column_mask(0x3);
   msg2->add_data(buffer);

   // Perform insert.
   auto resp2 = cp.process(request2);

   ASSERT_EQ(CommandResponse::INSERT, resp2.kind());
   ASSERT_TRUE(resp2.has_insert());
   ASSERT_EQ(txn_id, resp2.insert().transaction_id());
   ASSERT_EQ(1, resp2.insert().row_count());

}

TEST(CellCmdProcessorTest, CanExecuteFetch)
{
   using namespace lattice::cell;

   command_processor cp;

   cp.create_table("test_table_1",
      {
      new lattice::cell::column
         {
         lattice::cell::column::data_type::integer, "id", 4
         }, new lattice::cell::column
         {
         lattice::cell::column::data_type::bigint, "c1", 8
         },
      });

   CommandRequest request;

   request.set_kind(CommandRequest::PREPARE);

   auto* msg = request.mutable_prepare();
   msg->set_create_transaction(true);

   // Prepare for insert
   auto resp = cp.process(request);
   auto txn_id = resp.prepare().transaction_id();
   auto tbl_id = cp.get_database().get_table_id("test_table_1");
   auto t = cp.get_database().get_table(tbl_id);

   table::text_tuple_type text_data
      {
      "123", "1234567890"
      };

   std::string buffer;

   t->to_binary(
      {
      true, true
      }, text_data, buffer);

   // Create insert message
   CommandRequest request2;

   request2.set_kind(CommandRequest::INSERT);
   auto* msg2 = request2.mutable_insert();

   msg2->set_transaction_id(txn_id);
   msg2->set_table_id(tbl_id);
   msg2->set_column_mask(0x3);
   msg2->add_data(buffer);

   // Perform insert.
   auto resp2 = cp.process(request2);

   // Create fetch
   CommandRequest request3;

   request3.set_kind(CommandRequest::FETCH);

   auto* msg3 = request3.mutable_fetch();

}

