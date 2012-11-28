#include <coordinator/cpp/consensus.h>

namespace lattice {
namespace coordinator {

consensus::consensus()
{
}

void consensus::process(Configuration& msg)
{
   auto sequence = msg.sequence();
   auto pos = message_map.find(sequence);

   // If we don't know the sequence number, perform initial processing or
   // bail out.
   if (pos == message_map.end())
      {
         // If we don't know the sequence number, and it's
         // not a pre-prepare message, abort processing.
         if ( msg.kind() != Configuration::PRE_PREPARE)
            {
               return;
            }

         // Ensure that the message integrity is valid.

         // Create a new entry.
         pos = message_map.insert(
               std::make_pair(sequence, consensus_sequence_type()));

         pos->second.pre_prepare = msg;
         return;
      }



}

}
}
