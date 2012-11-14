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
