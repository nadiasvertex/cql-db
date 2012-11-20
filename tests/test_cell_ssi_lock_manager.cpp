#include <cstdint>
#include <memory>
#include <sstream>

#include <cell/cpp/ssi_lock_manager.h>
#include <gtest/gtest.h>

TEST(SsiLockManagerTest, CanCreate)
{
   using namespace lattice::cell;

   std::unique_ptr<ssi_lock_manager> lm;
   ASSERT_NO_THROW(
         lm = std::unique_ptr<ssi_lock_manager>(new ssi_lock_manager()));
}

TEST(SsiLockManagerTest, CanTrackRead)
{
   using namespace lattice::cell;

   ssi_lock_manager lm;

   transaction_id gen_tid;
   row_id gen_rid;
   page::object_id_type tbl_id = 1;

   auto tid = gen_tid.next();
   auto rid = gen_rid.next();

   ASSERT_NO_THROW(lm.track_read(tid, tbl_id, rid));
}


TEST(SsiLockManagerTest, IgnoresWriteAfterReadInSameTxn)
{
   using namespace lattice::cell;

   ssi_lock_manager lm;

   transaction_id gen_tid;
   row_id gen_rid;
   page::object_id_type tbl_id = 1;

   auto tid = gen_tid.next();
   auto rid = gen_rid.next();

   lm.track_read(tid, tbl_id, rid);

   ASSERT_FALSE(lm.track_write(tid, tbl_id, rid));
}

TEST(SsiLockManagerTest, CanTrackRWDependency)
{
   using namespace lattice::cell;

   ssi_lock_manager lm;

   transaction_id gen_tid;
   row_id gen_rid;
   page::object_id_type tbl_id = 1;

   auto tid1 = gen_tid.next();
   auto tid2 = gen_tid.next();

   auto rid = gen_rid.next();

   lm.track_read(tid1, tbl_id, rid);

   ASSERT_TRUE(lm.track_write(tid2, tbl_id, rid));
}

TEST(SsiLockManagerTest, CanDetectDangerousStructure1)
{
   using namespace lattice::cell;

   ssi_lock_manager lm;

   transaction_id gen_tid;
   row_id gen_rid;
   page::object_id_type tbl_id = 1;

   auto tid1 = gen_tid.next();
   auto tid2 = gen_tid.next();
   auto tid3 = gen_tid.next();

   auto rid1 = gen_rid.next();
   auto rid2 = gen_rid.next();

   // T2 reads rid1
   lm.track_read(tid2, tbl_id, rid1);
   // T3 writes rid1
   lm.track_write(tid3, tbl_id, rid1);
   // T1 reads rid1
   lm.track_read(tid1, tbl_id, rid1);
   // T1 reads rid2
   lm.track_read(tid1, tbl_id, rid2);
   // T2 writes rid2
   lm.track_write(tid2, tbl_id, rid2);

   auto results = lm.check_for_conflicts(tid3);
   ASSERT_TRUE(std::get<1>(results));
   ASSERT_EQ(tid2, std::get<0>(results));
}

TEST(SsiLockManagerTest, CanDetectDangerousStructure2)
{
   using namespace lattice::cell;

   ssi_lock_manager lm;

   transaction_id gen_tid;
   row_id gen_rid;
   page::object_id_type tbl_id = 1;

   auto tid1 = gen_tid.next();
   auto tid2 = gen_tid.next();

   auto rid1 = gen_rid.next();

   // T1 reads rid1
   lm.track_read(tid1, tbl_id, rid1);
   // T2 reads rid1
   lm.track_read(tid2, tbl_id, rid1);
   // T1 writes rid1
   lm.track_write(tid1, tbl_id, rid1);
   // T2 writes rid1
   lm.track_write(tid2, tbl_id, rid1);

   auto results = lm.check_for_conflicts(tid1);
   ASSERT_TRUE(std::get<1>(results));
   ASSERT_EQ(tid2, std::get<0>(results));
}

TEST(SsiLockManagerTest, CanResolveDangerousStructure)
{
   using namespace lattice::cell;

   ssi_lock_manager lm;

   transaction_id gen_tid;
   row_id gen_rid;
   page::object_id_type tbl_id = 1;

   auto tid1 = gen_tid.next();
   auto tid2 = gen_tid.next();
   auto tid3 = gen_tid.next();

   auto rid1 = gen_rid.next();
   auto rid2 = gen_rid.next();

   // T2 reads rid1
   lm.track_read(tid2, tbl_id, rid1);
   // T3 writes rid1
   lm.track_write(tid3, tbl_id, rid1);
   // T1 reads rid1
   lm.track_read(tid1, tbl_id, rid1);
   // T1 reads rid2
   lm.track_read(tid1, tbl_id, rid2);
   // T2 writes rid2
   lm.track_write(tid2, tbl_id, rid2);

   // Try to commit t3 (we expect to fail)
   auto results = lm.check_for_conflicts(tid3);
   ASSERT_TRUE(std::get<1>(results));

   // At this point, we simulate aborting the conflicting
   // transaction.
   lm.abort(std::get<0>(results));

   // Now try to commit t3 again (we expect to succeed)
   results = lm.check_for_conflicts(tid3);
   ASSERT_FALSE(std::get<1>(results));
}

TEST(SsiLockManagerTest, CanCollectAllTransactions)
{
   using namespace lattice::cell;

   ssi_lock_manager lm;

   transaction_id gen_tid;
   row_id gen_rid;
   page::object_id_type tbl_id = 1;

   auto tid1 = gen_tid.next();
   auto tid2 = gen_tid.next();
   auto tid3 = gen_tid.next();

   auto rid1 = gen_rid.next();
   auto rid2 = gen_rid.next();

   // T2 reads rid1
   lm.track_read(tid2, tbl_id, rid1);
   // T3 writes rid1
   lm.track_write(tid3, tbl_id, rid1);
   // T1 reads rid1
   lm.track_read(tid1, tbl_id, rid1);
   // T1 reads rid2
   lm.track_read(tid1, tbl_id, rid2);
   // T2 writes rid2
   lm.track_write(tid2, tbl_id, rid2);

   // Simulate aborting the conflicting transaction.
   lm.abort(tid2);
   // Simulate committing the remaining transactions.
   lm.commit(tid3);
   lm.commit(tid1);

   ASSERT_EQ(0, lm.writer_graph_size());
}

