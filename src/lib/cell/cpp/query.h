#ifndef __LATTICE_CELL_QUERY_H__
#define __LATTICE_CELL_QUERY_H__

#include <vector>

#include <cell/cpp/predicate.h>
#include <cell/cpp/database.h>

namespace lattice
{
namespace cell
{

class query
{
public:
  /**
   * Binds a cursor to a predicate.
   */
  typedef struct
  {
    /**
     * The predicate to execute.
     */
    predicate_handle_type predicate;

    /**
     * The cursor to execute against.
     */
    page_cursor& cursor;

  } predicate_group_type;

  /**
   * Contains a list of predicates.
   */
  typedef std::vector<predicate_group_type> predicate_group_list_type;

private:
  database& db;

  predicate_group_list_type predicates;

public:
  query(database& _db) :
      db(_db)
  {
  }

};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_QUERY_H__
