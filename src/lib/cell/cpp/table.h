#ifndef __LATTICE_CELL_TABLE_H__
#define __LATTICE_CELL_TABLE_H__

#include <array>
#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>

#include <cell/cpp/unstringify.h>
#include <cell/cpp/row_id.h>
#include <cell/cpp/row_value.h>
#include <cell/cpp/page.h>

namespace lattice {
namespace cell {

class table
{
public:
   /** Indicates various results that a row fetch can issue. */
   enum class fetch_code
   {
      SUCCESS,          // The fetch worked.

      DOES_NOT_EXIST,   // No such row

      ISOLATED,         // The row exists, but you are not allowed to view
                        // it because of your current transactional constraints.

      OVER_FLOW,        // The buffer passed was not large enough to receive
                        // the requested data.

      CORRUPT_PAGE,     // The page containing the column recorded a data
                        // offset that was not valid.

      UNKNOWN_DATA_TYPE // The storage engine does not know how to read the
                        // data specified in one or more columns.

   };

   /** Indicates various results that a row insert can issue. */
   enum class insert_code
   {
      SUCCESS,          // The fetch worked.

      UNDER_FLOW,       // The buffer passed did not have as much data as
                        // the call claimed.

      OUT_OF_MEMORY,    // Unable to insert data because there is not
                        // enough memory to allocate the structures to
                        // hold the data.

      UNKNOWN_DATA_TYPE // The storage engine does not know how to store the
                        // data specified in one or more columns.
   };

   /**
    * A row contains pointers to the data for all columns stored in a row.
    */
   typedef row_value row_type;

   /**
    * The column data for the table.
    */
   typedef std::vector<page_handle_type> column_data_type;

   /**
    * A hash of rows.
    */
   typedef std::unordered_map<row_id, row_type, row_id_hash> row_list_type;

   /**
    * Indicates if a column is present in a data operation.
    */
   typedef std::vector<bool> column_present_type;

   /**
    * Provides storage for text results read from a command string.
    */
   typedef std::vector<std::string> text_tuple_type;

private:
   /**
    * The list of rows assigned to the table.
    */
   row_list_type rows;

   /**
    * The column data for this table.
    */
   column_data_type column_data;

   /**
    * The column name to column index mapping.
    */
   std::unordered_map<std::string, unsigned int> column_names;

   /**
    * The number of columns in the table.
    */
   unsigned int number_of_columns;

   /**
    * The next row id to generate.
    */
   row_id row_id_generator;

   /**
    * The table id.
    */
   page::object_id_type table_id;

public:

   table(page::object_id_type _table_id, unsigned int _number_of_columns) :
            table_id(_table_id),
            number_of_columns(_number_of_columns)
   {
      for (auto i = 0; i < number_of_columns; ++i)
         {
            column_data.push_back(page_handle_type(nullptr));
         }
   }

   /**
    * Provides the table id.
    */
   page::object_id_type get_table_id() const
   {
      return table_id;
   }

   /**
    * Indicates how many columns are defined for
    * this table.
    */
   unsigned int get_number_of_columns()
   {
      return number_of_columns;
   }

   /**
    * Get the column id by name.
    *
    * @param name: The name of the column.
    */
   unsigned int get_column_id(const std::string& name)
   {
      auto pos = column_names.find(name);
      if (pos != column_names.end())
         {
            // Need to throw an exception here, because the
            // column name does not exist.
            return 0;
         }

      return pos->second;
   }

   /**
    * Provides a new, unique row id.
    */
   row_id get_next_row_id()
   {
      return row_id_generator.next();
   }

   /**
    * Set the column definition for the given column.
    *
    * @param column_number: The column to set.
    * @param col: The definition to write.
    *
    * @returns: true if it worked, false if it didn't.
    */
   bool set_column_definition(unsigned int column_number, column* col)
   {
      if (column_number >= number_of_columns || col == nullptr)
         {
            return false;
         }

      if (column_data[column_number].get() != nullptr)
         {
            // This means that we have already set the column definition
            // and overwriting it would cause data loss. In this case we
            // need to alter the definition so that we migrate the data
            // to the new format.
            //
            // This function is not currently implemented, so return false.
            return false;
         }

      // Update the name map.
      if (column_names.find(col->name) != column_names.end())
         {
            // The column name already exists.
            return false;
         }

      // Create a new column and set the definition.
      column_data[column_number] = page_handle_type(new page(col));
      column_names[col->name] = column_number;

      return true;
   }

   /**
    * Provides an iterator pointing to the first row of this table.
    */
   row_list_type::iterator begin()
   {
      return rows.begin();
   }

   /**
    * Provides an iterator pointing to the row just after then end of
    * this table.
    */
   row_list_type::iterator end()
   {
      return rows.end();
   }

   /**
    * Insert the row into the table.
    *
    * @param tid: The transaction id to associate this insert with.
    * @param rid: A reference to a row_id object. This gets filled in
    *             with the row id assigned by the inserter.
    *
    * @param present: Each bit indicates whether the corresponding
    *                 column is present. Only present columns will
    *                 be read from the data buffer and inserted into
    *                 the column store.
    *
    * @param data: The data buffer to read data from.
    */
   insert_code insert_row(const transaction_id& tid, row_id& rid,
         const column_present_type& present, const std::string& data);

   /**
    * Commit a row to the table store.
    *
    * @param tid: The id of the committing transaction.
    * @param rid: The id of the row.
    *
    * This function essentially makes a row visible to other transactions. The
    * insert_row() call stages the data by writing the data into the various
    * column stores. All other transactions will ignore the data until it is
    * committed.
    *
    */
   bool commit_row(const transaction_id& tid, const row_id& rid);

   /**
    * Fetch a row from the table.
    *
    * @param pos: An iterator pointing to the a row.
    *
    * @param present: Each bit indicates whether the corresponding
    *                 column should be present. If the column is
    *                 marked present in this vector, it will be
    *                 read from the column store and written into
    *                 the buffer.
    *
    * @param buffer: The data buffer to write data into.
    */
   fetch_code fetch_row(const transaction_id& tid, row_list_type::iterator& pos,
         const column_present_type& present, std::ostream& buffer);

   /**
    * Fetch a row from the table.
    *
    * @param tid: The transaction id for the fetch context.
    * @param rid: The oid for the row.
    *
    * @param present: Each bit indicates whether the corresponding
    *                 column should be present. If the column is
    *                 marked present in this vector, it will be
    *                 read from the column store and written into
    *                 the buffer.
    *
    * @param buffer: The buffer to write data into.
    */
   fetch_code fetch_row(const transaction_id& tid, const row_id& rid,
         const column_present_type& present, std::ostream& buffer);

   /**
    * Converts a text tuple into a binary format.
    *
    * @param present: Each bit indicates whether the corresponding
    *                 column should be present. If the column is
    *                 marked present in this vector, it will be
    *                 read from the column store and written into
    *                 the buffer.
    *
    * @param tuple: The text tuple, a list of strings which need
    *               to be converted into a binary buffer.
    *
    * @param buffer: The data buffer to write data into.
    */
   bool to_binary(const column_present_type& present, const text_tuple_type& tuple,
         std::string& buffer);

};

/**
 * Tracks a table. May be shared among various objects.
 */
typedef std::shared_ptr<table> table_handle_type;

} // namespace cell
} // namespace lattice

#endif // __LATTICE_CELL_TABLE_H__
