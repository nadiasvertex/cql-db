#include "manager.h"

namespace lattice {
namespace group {

void manager::query_processor_thread()
{
   zmq::socket_t response(ctx, ZMQ_REP);
   response.bind("tcp://*:8");

   while (continue_processing)
      {
         zmq::message_t msg;
         response.recv(&msg);

         // We have received a single message. We process that message
         // in a single thread. Once the message has performed initial
         // processing, we reply with a batch of results, or we provide
         // an error.

         // We never reply to messages out of order. 0MQ doesn't allow
         // it, and that simplifies our life.




      }
}

} // end namespace lattice
} // end namespace group
