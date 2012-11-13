#include <sstream>
#include <cell/cpp/transaction.h>

namespace lattice {
namespace cell {

void transaction::create_version(table_handle_type t)
{
   auto tbl_id = t->get_table_id();
   auto pos = versions.find(tbl_id);

   if (pos == versions.end())
      {
         version_type version
            {
            t
            };
         pos = versions.insert(std::make_pair(tbl_id, version)).first;
      }

}

bool transaction::insert_columns(table_handle_type t, const std::string& data,
      const std::vector<bool>& present)
{
   auto tbl_id = t->get_table_id();
   auto pos = versions.find(tbl_id);
   if (pos == versions.end())
      {
         create_version(t);
         pos = versions.find(tbl_id);
      }

   auto& version = pos->second;

   row_id rid;

   if (t->insert_row(id, rid, present, data) != table::insert_code::SUCCESS)
      {
         return false;
      }

   version.added.insert(rid);
   return true;
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
               t->commit_row(id, row);
            }
      }

   return true;
}

bool transaction::fetch_columns(cursor_type &cursor, std::string& data,
      const std::vector<bool>& present)
{
   while (true)
      {
         // If the cursor is at the end, don't try to fetch.
         if (cursor.it == cursor.t->end())
            {
               return false;
            }

         std::stringstream out;

         switch (cursor.t->fetch_row(id, cursor.it, present, out, il))
            {
            case table::fetch_code::SUCCESS:    // return the data
               data = out.str();
               return true;

            case table::fetch_code::ISOLATED:   // go to the next row
               ++cursor.it;
            break;

            default:                            // error, no more data
               return false;
            }
      }
}

bool transaction::update_columns(cursor_type &cursor, std::string& data,
      const std::vector<bool>& present)
{

}

} // namespace cell
} // namespace lattice

