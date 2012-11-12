#include <sstream>
#include <vector>

#include <cell/cpp/command_processor.h>
#include <cell/cpp/data_value.h>

namespace lattice {
namespace cell {

page::object_id_type command_processor::create_transaction()
{
   auto txn_id = ++next_transaction_id;
   transactions.insert(std::make_pair(txn_id, transaction()));

   return txn_id;
}

page::object_id_type command_processor::create_cursor(
      page::object_id_type txn_id, page::object_id_type table_id)
{
   auto pos = transactions.find(txn_id);
   if (pos == transactions.end())
      {
         return 0;
      }

   // Get transaction
   auto& txn = pos->second;

   // Get the table
   auto t = db.get_table(table_id);

   // Create a cursor on the table.
   return txn.create_cursor(t);
}

std::string command_processor::fetch_columns(page::object_id_type txn_id,
      page::object_id_type cursor_id, std::vector<int> column_indexes)
{
   auto pos = transactions.find(txn_id);
   if (pos == transactions.end())
      {
         return std::string();
      }

   // Get transaction
   auto& txn = pos->second;

   // Get cursor
   auto& cursor = txn.get_cursor(cursor_id);
   auto& row = cursor.it;
   auto& t = cursor.t;

   // Temporary buffer for writing out data.
   std::stringstream out;
   std::vector<bool> present;

   present.assign(t->get_number_of_columns(), false);
   for (auto index : column_indexes)
      {
         present[index] = true;
      }

   t->fetch_row(row, present, out);
   return out.str();
}

bool command_processor::insert_columns(page::object_id_type txn_id,
      page::object_id_type table_id, std::vector<int> column_indexes,
      const std::string& data)
{
   auto pos = transactions.find(txn_id);
   if (pos == transactions.end())
      {
         return false;
      }

   auto& txn = pos->second;           // Get transaction
   auto t = db.get_table(table_id);   // Get table

   std::vector<bool> present;

   present.assign(t->get_number_of_columns(), false);
   for (auto index : column_indexes)
      {
         present[index] = true;
      }

   txn.insert_columns(t, data, present);
}

//                                                                           //
// ============------------ Command Processing -------------================ //
//                                                                           //

CommandResponse command_processor::insert(const CommandRequest& request,
      CommandResponse& resp)
{
   auto* insert_response = resp.mutable_insert();

   resp.set_kind(CommandResponse::INSERT);

   auto& msg = request.insert();
   auto txn_id = msg.transaction_id();
   auto tbl_id = msg.table_id();

   auto column_mask = msg.column_mask();
   std::vector<int> column_indexes;

   // Optimization note: check how many columns exist instead
   // of checking all 64 bits each time.
   for (auto b = 0; b < 64; ++b)
      {
         column_indexes.push_back(b);
      }

   // Loop over the data packets for this insert.
   for (auto j = 0; j < msg.data_size(); ++j)
      {
         insert_columns(txn_id, tbl_id, column_indexes, msg.data(j));
      }

   // In the future, actually check to see if this worked.
   insert_response->set_row_count(msg.data_size());

   return resp;
}

CommandResponse command_processor::fetch(const CommandRequest& request,
      CommandResponse& resp)
{
   auto* fetch_response = resp.mutable_fetch();

   resp.set_kind(CommandResponse::FETCH);

   auto& msg = request.fetch();
   auto txn_id = msg.transaction_id();

   for (auto i = 0; i < msg.cursors_size(); ++i)
      {
         auto cursor_id = msg.cursors(i);
         auto batch_size = msg.batch_size(i);
         auto column_mask = msg.column_mask(i);

         std::vector<int> column_indexes;

         // Optimization note: check how many columns exist instead
         // of checking all 64 bits each time.
         for (auto b = 0; b < 64; ++b)
            {
               column_indexes.push_back(b);
            }

         // Fetch the batch for this cursor.
         fetch_response->add_cursors(cursor_id);
         for (auto j = 0; j < batch_size; ++j)
            {
               fetch_response->add_data(
                     fetch_columns(txn_id, cursor_id, column_indexes));
            }

         fetch_response->add_batch_size(batch_size);
      }

   return resp;
}

CommandResponse command_processor::prepare(const CommandRequest& request,
      CommandResponse& resp)
{
   auto* prepare_response = resp.mutable_prepare();

   resp.set_kind(CommandResponse::PREPARE);

   auto& msg = request.prepare();

   // Establish the transaction id.
   page::object_id_type txn_id;
   if (msg.has_create_transaction() && msg.create_transaction())
      {
         txn_id = create_transaction();
         prepare_response->set_transaction_id(txn_id);
      }
   else
      {
         txn_id = msg.transaction_id();
      }

   // Create all requested cursors.
   for (auto i = 0; i < msg.cursors_size(); ++i)
      {
         auto table_id = db.get_table_id(msg.cursors(i));
         auto cursor_id = create_cursor(txn_id, table_id);
         prepare_response->add_cursor_ids(cursor_id);
      }

   return resp;
}

CommandResponse command_processor::process(const CommandRequest& request)
{
   CommandResponse resp;

   switch (request.kind())
      {
      case CommandRequest::PREPARE:
         return prepare(request, resp);
      break;
      case CommandRequest::FETCH:
         return fetch(request, resp);
      break;
      }

   return resp;
}

} // namespace cell
} // namespace lattice

