#include <cell/cpp/table.h>

namespace lattice
{
namespace cell
{

bool table::insert_row(page::object_id_type row_id, column_present_type present,
    std::uint8_t *buffer, std::size_t buffer_size)
{
  std::size_t offset = 0;
  row_type row;

  for (auto i = 0; i < number_of_columns; ++i)
    {
      // If the column is not present, don't try to write it.
      if (present.size() <= i || present[i] == false)
        {
          row.push_back(0);
          continue;
        }

      // Abort if we are out of buffer space.
      if (buffer_size <= offset)
        {
          return false;
        }

      // Fetch the bits of information we need to write
      // the data properly.
      auto p = column_data[i].get();
      auto c = p->get_column_definition();
      auto oid = p->get_next_oid();

      row.push_back(oid);

      switch (c->type)
        {

#include "row_insert_int_ops.h"

        default:
          return false;
        }
    }

  rows.insert(
        {
        row_id, std::move(row)
        });
  return true;
}

bool table::fetch_row(page::object_id_type row_id, column_present_type present,
    std::uint8_t *buffer, std::size_t buffer_size)
{
  std::size_t offset = 0;

  // See if the row exists.
  auto pos = rows.find(row_id);
  if (pos == rows.end())
    {
      return false;
    }

  // Get a reference to the row.
  auto& row = pos->second;

  // Read columns from the row as requested.
  for (auto i = 0; i < number_of_columns; ++i)
    {
      // If the column is not present, don't try to write it.
      if (present.size() <= i || present[i] == false)
        {
          continue;
        }

      // Abort if we are out of buffer space.
      if (buffer_size <= offset)
        {
          return false;
        }

      // Fetch the bits of information we need to read
      // the data properly.
      auto p = column_data[i].get();
      auto c = p->get_column_definition();
      auto oid = row[i];

      switch (c->type)
        {

#include "row_fetch_int_ops.h"

        default:
          return false;
        }
    }

  return true;
}

} // end namespace cell
} // end namespace lattice
