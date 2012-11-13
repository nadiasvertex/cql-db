#ifndef __LATTICE_CELL_ROW_VALUE_H__
#define __LATTICE_CELL_ROW_VALUE_H__

#include <cstdint>
#include <vector>

#include <cell/cpp/transaction_id.h>
#include <cell/cpp/page.h>

namespace lattice {
namespace cell {

class row_value
{
public:
   typedef std::uint8_t column_count_type;

private:
   /** The transaction id wherein this row was last written.*/
   transaction_id transaction_write_id;

   /** Indicates who is updating this row. If non-zero, this row is locked for
    * writes by other transaction. */
   transaction_id transaction_lock_id;

   /** The number of columns in the row. */
   column_count_type number_of_columns;

   /** Indicates if the row was actually committed. If not, this row is
    * only visible to the transaction that wrote it.
    */
   bool committed;

   /** The array of column ids. */
   page::object_id_type *column_oids;

public:
   /**
    * Creates a new row, initializes it with data.
    *
    * @param _write_id: The transaction writing this row.
    * @param init: The data to initialize the row with.
    *
    * @note: The row is created in a locked state. It will
    * be unlocked when commit() is called.
    */
   row_value(transaction_id _write_id,
         const std::vector<page::object_id_type>& init) :
            transaction_write_id(_write_id),
            transaction_lock_id(_write_id),
            number_of_columns(init.size()),
            committed(false)
   {
      column_oids = new page::object_id_type[number_of_columns];
      for (auto i = 0; i < number_of_columns; ++i)
         {
            column_oids[i] = init[i];
         }
   }
   ;

   /**
    * Copy constructor - duplicates the tuple.
    */
   row_value(const row_value& o) :
            transaction_write_id(o.transaction_write_id),
            transaction_lock_id(o.transaction_lock_id),
            number_of_columns(o.number_of_columns),
            committed(o.committed)
   {
      column_oids = new page::object_id_type[number_of_columns];
      for (auto i = 0; i < number_of_columns; ++i)
         {
            column_oids[i] = o.column_oids[i];
         }
   }

   ~row_value()
   {
      if (column_oids != nullptr)
         {
            delete[] column_oids;
         }
   }

   /**
    * Gets the oid for the requested column. A value
    * of zero means that there is no data in that column. (NULL)
    */
   page::object_id_type column(column_count_type idx)
   {
      if (idx < number_of_columns)
         {
            return column_oids[idx];
         }

      return 0;
   }

   /**
    * Marks this row as committed, and updates the write id.
    *
    *
    * @param: txn_id: The transaction id to commit.
    *
    * @returns: true if the transaction committed, false otherwise.
    *
    * @note: Automatically unlocks the rows.
    */
   bool commit(transaction_id txn_id)
   {
      if (unlock(txn_id))
         {

            committed = true;
            transaction_write_id = transaction_lock_id;
            return true;
         }

      return false;
   }

   /**
    * Determines if this row is visible to the given transaction.
    *
    * @param txn_id: The transaction id requested.
    *
    * @returns: true if the row is visible to txn_id, false otherwise.
    */
   bool is_visible(const transaction_id& txn_id) const
   {
      if (committed)
         {
            return transaction_write_id < txn_id;
         }

      return transaction_lock_id == txn_id;
   }

   /**
    * Locks this row for writing.
    *
    * @param txn_id: The transaction that wants to lock the row.
    *
    * @returns: true if the lock succeeded, false otherwise.
    */
   bool lock(const transaction_id& txn_id)
   {
      if (transaction_lock_id.empty())
         {
            transaction_lock_id = txn_id;
            return true;
         }

      if (transaction_lock_id == txn_id)
         {
            return true;
         }

      return false;
   }

   /**
    * Unlocks this row for writing. Only the transaction which locked the
    * row may unlock it.
    *
    * @param txn_id: The transaction that wishes to unlock this row.
    *
    * @returns: true if the unlock worked, false otherwise.
    */
   bool unlock(const transaction_id& txn_id)
   {
      if (transaction_lock_id.empty())
         {
            return true;
         }

      if (transaction_lock_id == txn_id)
         {
            transaction_lock_id.reset();
            return true;
         }

      return false;
   }

   bool operator==(const row_value& o) const
   {
      for (auto i = 0; i < number_of_columns; ++i)
         {
            if (column_oids[i] != o.column_oids[i])
               {
                  return false;
               }
         }

      return true;
   }

   bool operator<(const row_value& o) const
   {
      for (auto i = 0; i < number_of_columns; ++i)
         {
            if (column_oids[i] >= o.column_oids[i])
               {
                  return false;
               }
         }

      return true;
   }

   /**
    * Move constructor - moves the tuple.
    */
   row_value(row_value&& o) :
   transaction_write_id(o.transaction_write_id),
   transaction_lock_id(o.transaction_lock_id),
   number_of_columns(o.number_of_columns),
   committed(o.committed)
      {
         column_oids = o.column_oids;
         o.column_oids = nullptr;
      }
};

} // namespace cell
         } // namespace lattice

#endif // __LATTICE_CELL_ROW_VALUE_H__
