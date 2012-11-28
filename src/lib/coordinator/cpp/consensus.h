#ifndef __LATTICE_COORDINATOR_CONSENSUS_H__
#define __LATTICE_COORDINATOR_CONSENSUS_H__

#include <queue>
#include <coordinator/proto/configuration.pb.h>

namespace lattice {
namespace coordinator {

/**
 * Implements the consensus algorithm for the cluster.
 */
class consensus
{
   typedef struct
   {
      Configuration pre_prepare;
      Configuration prepare;
      Configuration commit;

      int prepare_count;
      int commit_count;
   } consensus_sequence_type;

   typedef std::queue<consensus_sequence_type> consensus_queue_type;

public:
   consensus();

};

bool operator<(const consensus::consensus_sequence_type& l,
      const consensus::consensus_sequence_type& r)
{
   return l.pre_prepare.sequence() < r.pre_prepare.sequence();
}

bool operator==(const consensus::consensus_sequence_type& l,
      const consensus::consensus_sequence_type& r)
{
   return l.pre_prepare.sequence() == r.pre_prepare.sequence();
}

}
}

#endif // __LATTICE_COORDINATOR_CONSENSUS_H__
