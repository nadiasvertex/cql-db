#ifndef __LATTICE_EDGE_CLIENT_REQUEST_H__
#define __LATTICE_EDGE_CLIENT_REQUEST_H__

#include <cstdint>

namespace lattice {
namespace edge {

class client_request_id
{
   std::uint64_t id;

   friend class client_request_id_hash;
public:
   client_request_id() :
         id(0)
   {
   }

   /**
    * Gets the next available id. Useful for generators.
    */
   client_request_id next()
   {
      return from_uint64(++id);
   }

   /**
    * Gets the id as a uint64.
    */
   std::uint64_t as_uint64() const
   {
      return id;
   }

   /**
    * Creates a request id from an existing uint64.
    *
    * @param _id: The existing id value (ie from a network packet.)
    */
   static client_request_id from_uint64(std::uint64_t _id)
   {
      client_request_id req_id;
      req_id.id = _id;

      return req_id;
   }

   bool operator==(const client_request_id& crid) const
   {
      return id == crid.id;
   }

   bool operator<(const client_request_id& crid) const
   {
      return id < crid.id;
   }
};

class client_request_id_hash
{
public:
   std::size_t operator()(const client_request_id& crid) const
   {
      std::hash<std::uint64_t> h;
      return h(crid.id);
   }
};

} // end namespace edge
} // end namespace lattice

#endif // __LATTICE_EDGE_CLIENT_REQUEST_H__
