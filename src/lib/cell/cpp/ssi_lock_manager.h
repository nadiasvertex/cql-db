#ifndef __LATTICE_CELL_SSI_LOCK_MANAGER_H__
#define __LATTICE_CELL_SSI_LOCK_MANAGER_H__

#include <tuple>
#include <unordered_map>
#include <unordered_set>
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

   typedef std::unordered_map<transaction_id, row_range_type,
         transaction_id_hash> transaction_range_map_type;

   typedef std::unordered_map<page::object_id_type, transaction_range_map_type> table_transaction_map_type;

   typedef std::unordered_set<transaction_id, transaction_id_hash> dependency_set_type;

   typedef std::unordered_map<transaction_id, dependency_set_type,
         transaction_id_hash> dependency_map_type;

   typedef std::size_t size_type;

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
    * Tracks concurrent transactions so that we know, for example T1 and T2
    * ran at the same time. Once all transactions from some slice of time
    * have committed or aborted we can tear down the read range maps.
    */
   dependency_map_type concurrent_transactions;

   /** The set of transactions which are currently active. */
   dependency_set_type active_transactions;

   /**
    * Adds a transaction to the list of concurrent transactions.
    *
    * @param tid: A new transaction to track.
    *
    * All current transactions are listed as concurrent to tid, and tid is
    * added as concurrent to all active transactions.
    */
   void add_transaction(const transaction_id& tid)
   {
      dependency_set_type concurrent;

      // If we already know about the transaction, don't add it again.
      if (active_transactions.insert(tid).second == false)
         {
            return;
         }

      // Create a mapping between the currently active transactions
      // and this new transaction.
      for (const auto& active_tid : active_transactions)
         {
            if (tid == active_tid)
               {
                  continue;
               }

            concurrent.insert(active_tid);
            concurrent_transactions.find(active_tid)->second.insert(tid);
         }

      concurrent_transactions.emplace(tid, std::move(concurrent));
   }

   /**
    * Deletes a transaction from the list of concurrent transactions.
    *
    * @param tid: The old transaction to stop tracking.
    *
    * Finds the existing transaction, then walks the set of transactions
    * that were concurrent. It removes itself as a dependency. If the
    * dependency is empty, that indicates that we can tear down the
    * read range records.
    */
   void del_transaction(const transaction_id& tid)
   {
      active_transactions.erase(tid);

      // Easy out, just clear everything.
      if (active_transactions.empty())
         {
            concurrent_transactions.clear();
            for(auto& txn_map : range_map)
               {
                  txn_map.second.clear();
               }

            dep_map.clear();
            return;
         }

      // Otherwise selectively clear out the system.
      auto txn_deps = concurrent_transactions.find(tid);
      if (txn_deps == concurrent_transactions.end())
         {
            return;
         }

      std::vector<transaction_id> empty;

      for (auto& concurrent_tid : txn_deps->second)
         {
            auto pos = concurrent_transactions.find(concurrent_tid);
            // If it wasn't found, then likely it has been committed
            // or aborted and removed.
            if (pos==concurrent_transactions.end())
               {
                  continue;
               }

            pos->second.erase(tid);

            if (pos->second.empty())
               {
                  empty.push_back(pos->first);
               }
         }

      for (auto& empty_tid : empty)
         {
            txn_deps->second.erase(empty_tid);
            concurrent_transactions.erase(empty_tid);
            // Tear down the read range locks.
            for (auto& pos : range_map)
               {
                  pos.second.erase(empty_tid);
               }
         }
   }

   /**
    * Creates an rw-antidependency mapping between the writer (T2) and the
    * reader (T1).
    *
    * @param writer: The transaction that wrote the row.
    * @param reader: The transaction that previously read the row.
    */
   void add_rw_dependency(const transaction_id& reader,
         const transaction_id& writer)
   {
      auto pos = dep_map.find(reader);
      if (pos == dep_map.end())
         {
            pos =
                  dep_map.insert(std::make_pair(reader, dependency_set_type())).first;
         }

      auto& dep_set = pos->second;
      dep_set.insert(writer);
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

   /**
    * When 'tid' is committed or rolled back, we need to delete the mapping
    * structures used for the read locks. We also collect the dependency
    * mappings.
    *
    * @param tid: The transaction being collected.
    * @param abort: If true, the collection will be processed as an
    *               abort, meaning that all writer information is
    *               removed (since an abort means that updates never
    *               actually happened.)
    */
   void collect(const transaction_id& tid, bool abort = false)
   {
      // Remove all dependencies to this transaction as a reader (since
      // we no longer care what it has read), but leave all references
      // to it as a writer (since other readers may still care.)
      dep_map.erase(tid);

      // Remove from active transactions.
      del_transaction(tid);
   }
public:
   /**
    * Provides the number of writer nodes that have outstanding reader
    * dependencies. Note that some of these writers may have committed
    *  already.
    */
   size_type writer_graph_size()
   {
      return dep_map.size();
   }

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
      add_transaction(tid);

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
      add_transaction(tid);

      // Find the table.
      auto table_pos = range_map.find(table_id);
      if (table_pos == range_map.end())
         {
            return false;
         }

      bool result = false;

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
                  result = true;
                  continue; // evaluate all transactions so that we find
                            // all dependencies.
               }
         }

      return result;
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
            // The transaction at t2 (T2) has an rw dependency on
            // tid (T3). Now check to see if T2 is an rw dependency of
            // some other transaction (T1).
            auto t1 = get_rw_dependency(t2);
            if (!t1.empty())
               {
                  // A dangerous structure has been located. Determine which
                  // transaction to terminate.

                  if (active_transactions.find(t2) != active_transactions.end())
                     {
                        return std::make_tuple(t2, true);
                     }

                  if (active_transactions.find(t1) != active_transactions.end())
                     {
                        return std::make_tuple(t1, true);
                     }

                  return std::make_tuple(tid, true);
               }
         }

      // No conflicts found
      return std::make_tuple(transaction_id(), false);
   }

   /**
    * When 'tid' is rolled back, we need to delete the mapping
    * structures used for the read locks. We also remove the write
    * mappings, because all writes done by the transaction are undone.
    *
    * @param tid: The transaction being aborted.
    */
   void abort(const transaction_id& tid)
   {
      collect(tid, true);
   }

   /**
    * When 'tid' is committed, we need to delete the mapping structures
    * used for the read locks. We don't remove writer dependency information
    * until all of its reader dependencies have been collected.
    *
    * @param tid: The transaction being collected.
    */
   void commit(const transaction_id& tid)
   {
      collect(tid, false);
   }

}
;

} // end namespace cell
} // end namespace lattice

#endif //__LATTICE_CELL_SSI_LOCK_MANAGER_H__
