#include <cell/cpp/transaction.h>

namespace lattice {
namespace cell {

bool transaction::insert_columns(table_handle_type t, const std::string& data,
      const std::vector<bool>& present)
{
   auto pos = versions.find(t->get_table_id());
   if (pos == versions.end())
      {
         version_type version
            {
            t
            };
         pos =
               versions.insert(std::make_pair(t->get_table_id(), version)).first;
      }

   auto& version = pos->second;

   auto row_id = t->get_next_row_id();
   auto vpos =
         version.added.insert(std::make_pair(row_id, table::row_type())).first;

   t->insert_row(vpos->second, present,
         static_cast<const uint8_t*>(static_cast<const void*>(data.c_str())),
         data.size());
}

bool transaction::commit()
{
   /**
    * Process each table version in turn.
    */
   for (auto& version : versions)
      {
         auto t = version.second.t;
         // Process all inserts.
         for (auto& row : version.second.added)
            {
               t->commit_row(row.first, row.second);
            }
      }

   return true;
}

} // namespace cell
} // namespace lattice

