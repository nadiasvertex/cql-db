#ifndef __LATTICE_COORDINATOR_CONSENSUS_H__
#define __LATTICE_COORDINATOR_CONSENSUS_H__

#include <cstdint>
#include <unordered_map>
#include <vector>
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

   typedef std::unordered_map<std::uint32_t, consensus_sequence_type> consensus_map_type;

   /** Map of messages to sequence types received. */
   consensus_map_type messages;

   /** The log of sequence numbers received. */
   std::vector<std::uint32_t> log;

public:
   consensus();

   void process(Configuration& msg);


};

}
}

#endif // __LATTICE_COORDINATOR_CONSENSUS_H__
