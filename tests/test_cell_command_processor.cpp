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

