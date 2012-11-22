#ifndef __LATTICE_EDGE_DISCOVERY_H__
#define __LATTICE_EDGE_DISCOVERY_H__

#include <string>
#include <apr-1/apr_network_io.h>


namespace lattice {
namespace edge {

class discovery
{
	apr_socket_t* broadcast;
	apr_sockaddr_t* broadcast_address;
	apr_pool_t *pool;

	std::string broadcast_msg;

	void handle_socket_error(apr_status_t status);

public:
   discovery(int port);

   void announce();

};

} // end namespace edge
} // end namespace lattice


#endif //__LATTICE_EDGE_DISCOVERY_H__
