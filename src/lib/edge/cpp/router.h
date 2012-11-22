#ifndef __LATTICE_EDGE_ROUTER_H__
#define __LATTICE_EDGE_ROUTER_H__

#include <zmq.hpp>

#include <edge/cpp/address.h>

namespace lattice {
namespace edge {

/**
 * The router provides the ability to broadcast, multicast, or 
 * unicast data between cells, or between cells and other objects
 * like planes or groups. The router maintains an edge to
 * every cell in the group, as well as two or more edges to every 
 * group in the lattice.
 *
 * There are two global, persistent planes. One is the control
 * plane, which provides a publish/subscribe model. Every
 * object in the lattice connects to the control plane. The
 * control planes for all groups in a lattice are also connected
 * together.
 *
 * The other global plane is the data plane. 
 */
class router
{
   zmq::context_t &ctx;

   address addr_gen;
public:
   router(zmq::context_t &_ctx);

   bool join_cluster();

};

} // end namespace edge
} // end namespace lattice

#endif // __LATTICE_EDGE_ROUTER_H__
