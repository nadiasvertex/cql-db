#ifndef __LATTICE_EDGE_DISCOVERY_H__
#define __LATTICE_EDGE_DISCOVERY_H__

#include <chrono>
#include <string>
#include <unordered_map>

#include <netinet/in.h>

#include <zmq.hpp>

namespace lattice {
namespace edge {

class discovery
{
	typedef struct
   {
		struct sockaddr_in addr;
      
		std::chrono::steady_clock::time_point check_in;

   } node_presence_type;

	typedef std::unordered_map<std::string, node_presence_type> node_map_type;

   /** The socket used for sending and receiving presence broadcasts. */
	int broadcast;
  
   /** The address bound for this node. */
   struct sockaddr_in si_this;

   /** The broadcast message to send for this node. */
	std::string broadcast_msg;

   /** Maps node uuid values to ip addresses. */
	node_map_type node_map;

   /**
    * Handles logging and reporting socket errors.
    *
    * @param msg: Part of the error message to deliver.
    * @param error: The error code to deliver.
    *
    */
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
