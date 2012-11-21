#ifndef __LATTICE_EDGE_CLIENT_PROCESSOR_H__
#define __LATTICE_EDGE_CLIENT_PROCESSOR_H__

#include <memory>
#include <unordered_map>

#include <edge/cpp/client_request_id.h>
#include <edge/proto/client_commands.pb.h>
#include <processor/cpp/query.h>

namespace lattice {
namespace edge {

class client_processor
{
   typedef std::unique_ptr<processor::query> query_handle_type;

   typedef std::unordered_map<client_request_id, query_handle_type,
         client_request_id_hash> request_map_type;

   /** These are the outstanding requests being
    * managed by this query processor.
    */
   request_map_type requests;

   /**
    * Generates client request ids.
    */
   client_request_id req_id_gen;

   /**
    * The metadata information for this client processor.
    */
   processor::metadata md;

   /**
    * Handles authentication requests.
    */
   ClientResponse authenticate(const ClientRequest& request);

   /** Handles query requests. */
   ClientResponse query(const ClientRequest& request);

public:

   ClientResponse process(const ClientRequest& request);
};

} // end namespace edge
} // end namespace lattice

#endif // __LATTICE_EDGE_CLIENT_PROCESSOR_H__
