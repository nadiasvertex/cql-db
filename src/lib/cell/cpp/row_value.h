#ifndef __LATTICE_CELL_ROW_VALUE_H__
#define __LATTICE_CELL_ROW_VALUE_H__

#include <cstdint>
#include <utility>
#include <vector>

#include <cell/cpp/transaction_id.h>
#include <cell/cpp/page.h>

namespace lattice {
namespace cell {

/**
 * Indicates if a column is present in a data operation.
 */
typedef std::vector<bool> column_present_type;

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

   /** This field does double duty: if the row is outright deleted, this serves
    * as the tombstone marker so that the row can be garbage collected. If
    * the row is updated, it tells us at which point the row is no longer
    * visible. Once all transactions <= transaction_deleted_id have committed,
    * this row can be removed from the data store.
    */
   transaction_id transaction_deleted_id;

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
    * For every bit where the 'present' vector is false, it copies the column
    * information from the 'o' row.
    *
    * @param txn_id: The transaction id doing the update.
    * @param present: A vector of bool indicating if column data is present.
    * @param o: An older version of this row from which to copy missing columns.
    *
    * @returns: true if the update can occur, false if the row is locked by
    *           someone else.
    */
   bool update(const transaction_id& txn_id, const column_present_type& present,
         const row_value& o)
   {
      if (is_locked(txn_id))
         {
            return false;
         }

      for (auto i = 0; i < present.size(); ++i)
         {
            if (present[i] == false)
               {
                  column_oids[i] = o.column_oids[i];
               }
         }

      return true;
   }

   /**
    * Marks this row as deleted.
    *
    * @param: txn_id: The transaction id that deleted the row.
    *
    * @returns: true if the transaction committed, false otherwise.
    */
   bool remove(const transaction_id& txn_id)
   {
      if (is_locked(txn_id))
         {
            transaction_deleted_id = txn_id;
            return true;
         }

      return false;
   }

   /**
    * Marks this row as committed, and updates the write id.
    *
    * @param: txn_id: The transaction id to commit.
    *
    * @returns: true if the transaction committed, false otherwise.
    *
    * @note: Automatically unlocks the rows.
    */
   bool commit(const transaction_id& txn_id)
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
    * Determines if this row is visible to the given transaction using
    * snapshot isolation semantics. The query always sees the row
    * as it existed when the transaction began.
    *
    * @param txn_id: The transaction id requested.
    *
    * @returns: true if the row is visible to txn_id, false otherwise.
    */
   bool is_snapshot_visible(const transaction_id& txn_id) const
   {
      using namespace std::rel_ops;

      // If the row is committed its write id must be
      // <= the current transaction id, AND the row
      // must not be deleted, or must have been deleted
      // in a transaction > the current transaction id.
      if (committed)
         {
            return transaction_write_id <= txn_id
                  && (transaction_deleted_id.empty()
                        || transaction_deleted_id > txn_id);
         }

      return transaction_lock_id == txn_id && transaction_deleted_id.empty();
   }

   /**
    * Determines if this row is visible to the given transaction using
    * read committed isolation semantics. The query always sees the latest
    * committed row.
    *
    * @param txn_id: The transaction id requested.
    *
    * @returns: true if the row is visible to txn_id, false otherwise.
    */
   bool is_visible(const transaction_id& txn_id) const
   {
      using namespace std::rel_ops;

      // If the row is committed its write id must be
      // <= the current transaction id, AND the row
      // must not be deleted.
      if (committed)
         {
            return transaction_write_id <= txn_id
                  && transaction_deleted_id.empty();

         }

      return transaction_lock_id == txn_id && transaction_deleted_id.empty();
   }

   /**
    * Determines if this row is locked for this transaction.
    *
    * @param txn_id: The transaction id requested.
    *
    * @returns: true if the row is locked to txn_id, false otherwise.
    */
   bool is_locked(const transaction_id& txn_id) const
   {
      if (transaction_lock_id.empty())
         {
            return false;
         }

      return !(transaction_lock_id == txn_id);
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
