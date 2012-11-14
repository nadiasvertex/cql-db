#ifndef __LATTICE_CELL_SSI_LOCK_MANAGER_H__
#define __LATTICE_CELL_SSI_LOCK_MANAGER_H__

#include <map>
#include <set>
#include <tuple>
#include <unordered_map>
#include <utility>

#include <cell/cpp/row_id.h>
#include <cell/cpp/transaction_id.h>
#include <cell/cpp/page.h>
#include <common/cpp/range.h>

namespace lattice {
namespace cell {

class ssi_lock_manager
{
   typedef common::range<row_id> row_range_type;

   typedef std::map<transaction_id, row_range_type> transaction_range_map_type;

   typedef std::unordered_map<page::object_id_type, transaction_range_map_type> table_transaction_map_type;

   typedef std::set<transaction_id> dependency_set_type;

   typedef std::map<transaction_id, dependency_set_type> dependency_map_type;

   /**
    * Holds the range locks so that we can determine when
    * a read-after-write conflict has occurred.
    */
   table_transaction_map_type range_map;

   /**
    * Tracks read-before-write conflicts to locate
    * dangerous structures.
    */
   dependency_map_type dep_map;

   /**
    * Creates an rw-antidependency mapping between the writer (T2) and the
    * reader (T1).
    *
    * @param writer: The transaction that wrote the row.
    * @param reader: The transaction that previously read the row.
    */
   void add_rw_dependency(const transaction_id& writer,
         const transaction_id& reader)
   {
      auto pos = dep_map.find(writer);
      if (pos == dep_map.end())
         {
            pos =
                  dep_map.insert(std::make_pair(writer, dependency_set_type())).first;
         }

      auto& dep_set = pos->second;
      dep_set.insert(reader);
   }

   /**
    * Checks to see if an rw-antidependency mapping between tid (T2)
    * and some other transaction (T1). If there is a mapping T1 -rw-> T2
    * then this function will return T1.
    *
    * @param tid: The transaction id of a writer checking to see if
    *             a reader has an rw-antidependency relationship.
    *
    * @returns: The transaction id of the reader (T1), or an empty
    *           transaction id.
    */
   transaction_id get_rw_dependency(const transaction_id& tid)
   {
      for (auto pos = dep_map.begin(); pos != dep_map.end(); ++pos)
         {
            // Don't bother checking if we are looking in our
            // own bucket.
            if (pos->first == tid)
               {
                  continue;
               }

            auto& s = pos->second;
            if (s.find(tid) != s.end())
               {
                  // The transaction at pos->first has an rw dependency on
                  // tid.
                  return pos->first;
               }
         }

      return transaction_id();
   }
public:
   /**
    * Tracks a read to a particular row in a particular table for some transaction.
    *
    * @param tid: The transaction executing the write.
    * @param table_id: The table being written.
    * @param rid: The row being written.
    */
   void track_read(const transaction_id& tid, page::object_id_type table_id,
         const row_id& rid)
   {
      // Find the table
      auto table_pos = range_map.find(table_id);
      if (table_pos == range_map.end())
         {
            table_pos = range_map.insert(
                  std::make_pair(table_id, transaction_range_map_type())).first;
         }

      // Find the transaction.
      auto& txn_map = table_pos->second;
      auto tid_pos = txn_map.find(tid);
      if (tid_pos == txn_map.end())
         {
            tid_pos =
                  txn_map.insert(std::make_pair(tid, row_range_type())).first;
         }

      // Add the row.
      auto& row_range = tid_pos->second;
      row_range.insert(rid);
   }

   /**
    * Tracks a write to a particular row in a particular table for some transaction.
    *
    * @param tid: The transaction executing the write.
    * @param table_id: The table being written.
    * @param rid: The row being written.
    *
    * @returns: true if the row has been read by some _other_ transaction, thus
    *           indicating a write-after-read dependency. Otherwise false.
    */
   bool track_write(const transaction_id& tid, page::object_id_type table_id,
         const row_id& rid)
   {
      // Find the table.
      auto table_pos = range_map.find(table_id);
      if (table_pos == range_map.end())
         {
            return false;
         }

      // Walk the transactions
      auto& txn_map = table_pos->second;
      for (auto &txn : txn_map)
         {
            // Ignore writes after reads in the same transaction.
            if (tid == txn.first)
               {
                  continue;
               }

            // If this transaction has read the same row we just wrote,
            // then we know that there is a write-after-read dependency here.
            if (txn.second.contains(rid))
               {
                  add_rw_dependency(txn.first, tid);
                  return true;
               }
         }

      return false;
   }

   /**
    * Checks to see if there are any dangerous structures prior to
    * the commit of 'tid'.
    *
    * @param tid: The transaction id of the transaction that is
    *             committing.
    *
    * @returns: A tuple of (abort_txn_id, conflicts_detected). If the
    *           conflicts_detected member is 'true', then abort_txn_id will
    *           contain the id of the transaction that must be aborted.
    *
    * @note: Serializability theory holds that if:
    *
    *                T1 -rw-> T2 -rw-> T3
    *
    *        AND if T3 is the first to commit, then this is a dangerous
    *        structure and may be involved in a serialization cycle. Ideally
    *        T2 would be aborted, T3 would commit, and T1 would get left
    *        alone.
    */
   std::tuple<transaction_id, bool> check_for_conflicts(
         const transaction_id& tid)
   {
      auto t2 = get_rw_dependency(tid);
      if (!t2.empty())
         {
            // The transaction at pos->first (T2) has an rw dependency on
            // tid (T3). Now check to see if T2 is an rw dependency of
            // some other transaction (T1).
            auto t1 = get_rw_dependency(t2);
            if (!t1.empty())
               {
                  // A dangerous structure has been located.
                  return std::make_tuple(t2, true);
               }
         }

      // No conflicts found
      return std::make_tuple(transaction_id(), false);
   }

}
;

} // end namespace cell
} // end namespace lattice

#endif //__LATTICE_CELL_SSI_LOCK_MANAGER_H__
