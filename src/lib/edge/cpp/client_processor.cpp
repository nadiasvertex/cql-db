#include <edge/cpp/client_processor.h>

namespace lattice {
namespace edge {

ClientResponse client_processor::authenticate(const ClientRequest& request)
{
   ClientResponse resp;

   resp.set_kind(ClientResponse::AUTHENTICATE_SUCCESS);

   resp.set_request_id(0);

   return resp;
}

ClientResponse client_processor::query(const ClientRequest& request)
{
   ClientResponse resp;
   auto id = req_id_gen.next();

   resp.set_kind(ClientResponse::QUERY_BATCH);
   resp.set_request_id(id.as_uint64());

   auto& q = request.query();
   auto qp = new processor::query(md, q.data());

   requests.insert(std::make_pair(id, query_handle_type(qp)));

   //auto row = qp->fetch_one();
}

ClientResponse client_processor::process(const ClientRequest& request)
{
   switch (request.kind())
      {
      case ClientRequest::AUTHENTICATE:
         return authenticate(request);
      break;

      case ClientRequest::QUERY:
         return query(request);
      break;

      case ClientRequest::NEXT_BATCH:
      break;
      }
}

} // end namespace edge
} // end namespace lattice

