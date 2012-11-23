#ifndef __LATTICE_EDGE_DISCOVERY_H__
#define __LATTICE_EDGE_DISCOVERY_H__

#include <string>
#include <netinet/in.h>

#include <zmq.hpp>

namespace lattice {
namespace edge {

class discovery
{
	int broadcast;
   struct sockaddr_in si_this;

	std::string broadcast_msg;

	void handle_socket_error(const std::string& msg, int error);

public:
   discovery(int port);
   ~discovery();

   /** Announce our presence to the cluster. */
   void announce();

   /** Listen for other announcements. */
   bool listen();

};

} // end namespace edge
} // end namespace lattice


#endif //__LATTICE_EDGE_DISCOVERY_H__
