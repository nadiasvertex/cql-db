#ifndef __LATTICE_CELL_TABLE_H__
#define __LATTICE_CELL_TABLE_H__

#include <array>
#include <string>
#include <unordered_map>

#include <cell/cpp/page.h>

namespace lattice
{
namespace cell
{

template<int number_of_columns>
class table
{
 public:
  /**
   * A row contains pointers to the data for all columns stored in a row.
   */
  typedef std::array<page::object_id_type, number_of_columns> row_type;

  /**
   * The column data is stored in a fixed size array for the table. Thus,
   * altering a table requires creating a new table object and moving the
   * important data from the old table to the new table.
   */
  typedef std::array<page_handle_type, number_of_columns> column_data_type;

  /**
   * A hash of rows.
   */
  typedef std::unordered_map<page::object_id_type, row_type> row_list_type;


 private:
  /**
   * The list of rows assigned to the table.
   */
  row_list_type rows;

  /**
   * The column data for this table.
   */
  column_data_type column_data;

 public:

  table() {

  }

  /**
   * Set the column definition for the given column.
   *
   * @param column_number: The column to set.
   * @param col: The definition to write.
   *
   * @returns: true if it worked, false if it didn't.
   */
  bool set_column_definition(unsigned int column_number, const column& col) {
    if (column_number >= column_data.size()) {
        return false;
    }

    if (column_data[column_number].get()!=nullptr) {
        // This means that we have already set the column definition
        // and overwriting it would cause data loss. In this case we
        // need to alter the definition so that we migrate the data
        // to the new format.
        //
        // This function is not currently implemented, so return false.
        return false;
    }

    // Copy the existing definition
    auto col_def = column_handle_type(new column(col));

    // Create a new column and set the definition.
    column_data[column_number] = page_handle_type(new page(col_def));

    return true;
  }



  
};

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_TABLE_H__
