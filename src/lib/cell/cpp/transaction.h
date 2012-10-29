#ifndef __LATTICE_CELL_TRANSACTION_H__
#define __LATTICE_CELL_TRANSACTION_H__

#include <unordered_map>
#include <unordered_set>

#include <cell/cpp/table.h>

namespace lattice
{
namespace cell
{

class transaction {
  typedef std::unordered_set<page::object_id_type> row_id_list_type;

  typedef struct {
    /** Reference to the table adjusted. */
    table_handle_type t;

    /** List of rows added during this transaction. */
    row_id_list_type added;

    /** List of rows updated during this transaction. */
    row_id_list_type updated;

    /** List of rows deleted during this transaction. */
    row_id_list_type deleted;

  } version_type;

  std::unordered_map<page::object_id_type, version_type> versions;

};


} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_TRANSACTION_H__
