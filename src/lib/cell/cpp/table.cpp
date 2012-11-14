#include <cstdlib>
#include <cell/cpp/data_value.h>
#include <cell/cpp/table.h>

namespace lattice {
namespace cell {

static inline bool row_is_visible(const transaction_id& tid,
      const table::row_type& row, isolation_level level)
{
   // Check to see if the row is visible to this transaction.
   switch (level)
      {
      case isolation_level::READ_COMMITTED:
         if (!row.is_visible(tid))
            {
               return false;
            }
      break;

      case isolation_level::REPEATABLE_READ:
         if (!row.is_snapshot_visible(tid))
            {
               return false;
            }
      break;
      }

   return true;
}

table::insert_code table::insert_row(const transaction_id& tid, row_id& rid,
      const column_present_type& present, const std::string& data)
{
   std::size_t offset = 0;
   auto buffer_size = data.size();
   auto* buffer =
         static_cast<const std::uint8_t*>(static_cast<const void*>(data.c_str()));

   std::vector<page::object_id_type> row_data;

   rid = get_next_row_id();

   for (auto i = 0; i < number_of_columns; ++i)
      {
         // If the column is not present, don't try to write it.
         if (present.size() <= i || present[i] == false)
            {
               row_data.push_back(0);
               continue;
            }

         // Abort if we are out of buffer space.
         if (buffer_size <= offset)
            {
               return insert_code::UNDER_FLOW;
            }

         // Fetch the bits of information we need to write
         // the data properly.
         auto p = column_data[i].get();
         auto c = p->get_column_definition();
         auto oid = p->get_next_oid();

         row_data.push_back(oid);

         switch (c->type)
            {

#include "row_insert_int_ops.h"

            default:
               return insert_code::UNKNOWN_DATA_TYPE;
            }
      }

   // Insert the data into the row buffer.
   auto result = rows.insert(std::make_pair(rid, row_type(tid, row_data)));

   return insert_code::SUCCESS;
}

bool table::commit_row(const transaction_id& tid, const row_id& rid)
{
   auto pos = rows.find(rid);
   if (pos != rows.end())
      {
         pos->second.commit(tid);
         return true;
      }
   return false;
}

table::fetch_code table::fetch_row(const transaction_id& tid,
      row_list_type::iterator& pos, const column_present_type& present,
      std::ostream& buffer, isolation_level level)
{
   // Get a reference to the row.
   auto& row = pos->second;

   if (!row_is_visible(tid, row, level))
      {
         return fetch_code::ISOLATED;
      }

   // Read columns from the row as requested.
   for (auto i = 0; i < number_of_columns; ++i)
      {
         // If the column is not present, don't try to read it.
         if (present.size() <= i || present[i] == false)
            {
               continue;
            }

         // Fetch the bits of information we need to read
         // the data properly.
         auto& p = column_data[i];
         auto c = p->get_column_definition();
         auto oid = row.column(i);

         data_value dv(c->type);

         // Sync up the data value.
         auto seek = p->get_stream(oid);
         if (std::get<0>(seek) == false)
            {
               return fetch_code::CORRUPT_PAGE;
            }

         // Copy the data from the column store to the
         // output buffer.
         dv.copy(*std::get<1>(seek), buffer);
      }

   if (level==isolation_level::SERIALIZABLE && ssi_lm!=nullptr)
      {
         ssi_lm->track_read(tid, table_id, pos->first);
      }

   return fetch_code::SUCCESS;
}

table::fetch_code table::fetch_row(const transaction_id& tid, const row_id& rid,
      const column_present_type& present, std::ostream& buffer,
      isolation_level level)
{
   // See if the row exists.
   auto pos = rows.find(rid);
   if (pos == rows.end())
      {
         return fetch_code::DOES_NOT_EXIST;
      }

   return fetch_row(tid, pos, present, buffer, level);
}

table::update_code table::update_row(const transaction_id& tid,
      row_list_type::iterator& pos, const column_present_type& present,
      const std::string& buffer, row_id& new_rid, isolation_level level)
{
   // Get a reference to the row.
   auto& old_row = pos->second;

   if (!row_is_visible(tid, old_row, level))
      {
         return update_code::ISOLATED;
      }

   // If someone is already updating the row, abort. Otherwise lock it
   // ourselves.
   if (!old_row.lock(tid))
      {
         return update_code::CONFLICT;
      }

   row_id rid;
   switch (insert_row(tid, rid, present, buffer))
      {
      default:
         return update_code::UNEXPECTED_INSERT_ERROR;

      case insert_code::UNDER_FLOW:
         return update_code::UNDER_FLOW;

      case insert_code::UNKNOWN_DATA_TYPE:
         return update_code::UNKNOWN_DATA_TYPE;

      case insert_code::OUT_OF_MEMORY:
         return update_code::OUT_OF_MEMORY;

      case insert_code::SUCCESS:
      break;
      }

   // Find the row that was just inserted.
   pos = rows.find(rid);
   auto& new_row = pos->second;

   // Update it so that it has copies of all the
   // unchanged data.
   new_row.update(tid, present, old_row);

   // Delete the old row.
   old_row.remove(tid);

   // Track a write on the old row, in case anyone has read it.
   if (level==isolation_level::SERIALIZABLE && ssi_lm!=nullptr)
         {
            ssi_lm->track_write(tid, table_id, pos->first);
         }

   return update_code::SUCCESS;
}

table::update_code table::update_row(const transaction_id& tid,
      const row_id& rid, const column_present_type& present,
      const std::string& buffer, row_id& new_rid, isolation_level level)
{
   // See if the row exists.
   auto pos = rows.find(rid);
   if (pos == rows.end())
      {
         return update_code::DOES_NOT_EXIST;
      }

   return update_row(tid, pos, present, buffer, new_rid, level);

}

bool table::to_binary(const column_present_type& present,
      const text_tuple_type& tuple, std::string& buffer)
{
   std::stringstream out;

   for (auto i = 0; i < number_of_columns; ++i)
      {
         // If the column is not present, don't try to write it.
         if (present.size() <= i || present[i] == false)
            {
               continue;
            }

         // Fetch the bits of information we need to write
         // the data properly.
         auto p = column_data[i].get();
         auto c = p->get_column_definition();

         data_value dv;
         dv.set_value(c->type, tuple[i]);

         dv.write(out);
      }

   buffer = out.str();
   return true;
}

} // end namespace cell
} // end namespace lattice
