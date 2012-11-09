#ifndef __LATTICE_CELL_DATABASE_H__
#define __LATTICE_CELL_DATABASE_H__

#include <unordered_map>
#include <unordered_set>

#include <cell/cpp/transaction.h>

namespace lattice
{
namespace cell
{

class database
{
public:
  /**
   * Maps object ids to names.
   */
  typedef std::unordered_map<page::object_id_type, std::string> oid_name_map_type;

  typedef std::unordered_map<std::string, page::object_id_type> name_oid_map_type;

  /** Maps oids to tables. */
  typedef std::unordered_map<page::object_id_type, table_handle_type> table_map_type;

private:
  /**
   * The map of table ids to table handles.
   */
  table_map_type tables;

  /**
   * The map of table ids to table names.
   */
  oid_name_map_type table_names;

  name_oid_map_type table_oids;

  /**
   * The next table id to be provided.
   */
  page::object_id_type next_table_id;

public:
  database() :
      next_table_id(1)
  {
  }

  /**
   * Create a table.
   *
   * @param name: The name of the table.
   * @param columns: The column definitions for the table.
   *
   * @returns: true if it worked, false otherwise.
   */
  bool create_table(const std::string& name,
      std::vector<column*> columns)
  {

    auto oid = next_table_id++;
    table_handle_type t = std::make_shared < table > (columns.size());

    for (auto i = 0; i < columns.size(); ++i)
      {
        if (t->set_column_definition(i, columns[i]) == false)
          {
            return false;
          }
      }

    tables[oid] = t;
    table_names[oid] = name;
    table_oids[name] = oid;

    return true;
  }

  page::object_id_type get_table_id(const std::string& name)
  {
	  return table_oids[name];
  }

  /**
   * Get the table that corresponds to the given table id.
   *
   * @param table_id: The object id of the table.
   */
  table_handle_type get_table(page::object_id_type table_id)
  {
	  return tables[table_id];
  }

};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_DATABASE_H__
