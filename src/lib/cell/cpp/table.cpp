#include <cstdlib>
#include <cell/cpp/data_value.h>
#include <cell/cpp/table.h>

namespace lattice {
namespace cell {

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

   // Check to see if the row is visible to this transaction.
   if (level == isolation_level::READ_COMMITTED && !row.is_visible(tid))
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
