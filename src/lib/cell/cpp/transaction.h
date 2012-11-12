#ifndef __LATTICE_CELL_TRANSACTION_H__
#define __LATTICE_CELL_TRANSACTION_H__

#include <unordered_map>
#include <unordered_set>

#include <cell/cpp/table.h>

namespace lattice {
namespace cell {

class transaction
{
   typedef std::unordered_set<page::object_id_type> row_id_list_type;

   typedef struct
   {
      /** Reference to the table adjusted. */
      table_handle_type t;

      /** List of rows added during this transaction. */
      table::row_list_type added;

      /** List of rows updated during this transaction. */
      table::row_list_type updated;

      /** List of rows deleted during this transaction. */
      row_id_list_type deleted;

   } version_type;

   typedef struct
   {
      /** The iterator for this cursor. */
      table::row_list_type::iterator it;

      /** Reference to the table the cursor is attached to. */
      table_handle_type t;
   } cursor_type;

   /** The map of version information for this transaction. The key is the table
    * id.*/
   std::unordered_map<page::object_id_type, version_type> versions;

   /** Maps a cursor id to a cursor for a row. */
   std::unordered_map<page::object_id_type, cursor_type> row_cursor_map;

   /** Tracks cursor identifiers. */
   page::object_id_type next_cursor_id;

public:
   transaction() :
         next_cursor_id(0)
   {
   }
   ;

   /**
    * Creates a new cursor object attached to this transaction.
    *
    * @param t: A handle to the table the cursor is attached to.
    */
   page::object_id_type create_cursor(table_handle_type t)
   {
      auto cursor_id = ++next_cursor_id;
      row_cursor_map.insert(std::make_pair(cursor_id, cursor_type
         {
         t->begin(), t
         }));

      return cursor_id;
   }

   /**
    * Provides a reference to the cursor object identified
    * by the given cursor id.
    *
    * @param cursor_id: The id of the cursor to fetch.
    */
   cursor_type& get_cursor(page::object_id_type cursor_id)
   {
      return row_cursor_map[cursor_id];
   }

   /**
    * Insert a new row into a table. The row will not be visible in the table
    * until the transaction commits.
    */
   bool insert_columns(table_handle_type t, const std::string& data,
         const std::vector<bool>& present);

   /**
    * Moves modifications into the table store.
    */
   bool commit();
};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_TRANSACTION_H__
