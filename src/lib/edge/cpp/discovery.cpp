#include <cstring>
#include <memory>

#include <arpa/inet.h>
#include <sys/select.h>

#include <apr-1/apr_uuid.h>
#include <log4cxx/logger.h>

#include <edge/cpp/discovery.h>
#include <edge/proto/discovery.pb.h>

using namespace log4cxx;

static LoggerPtr logger(Logger::getLogger("cql.edge"));

namespace lattice {
namespace edge {

void discovery::handle_socket_error(const std::string& msg, int error)
{
   LOG4CXX_ERROR(logger, "Discovery: "
         << msg
         << " '"
         << std::strerror(error)
         << "'");
}

discovery::discovery(int port)
{
   //  Create UDP socket
   if ((broadcast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
      {
         handle_socket_error("creating broadcast socket", errno);
         return;
      }

   //  Ask operating system to let us do broadcasts from socket
   int on = 1;
   if (setsockopt(broadcast, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) == -1)
      {
         handle_socket_error("requesting broadcast permissions", errno);
         return;
      }
   
   // Allow reuse of the socket
   if (setsockopt(broadcast, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
      {
         handle_socket_error("requesting socket multiplex", errno);
         return;
      }


   //  Bind UDP socket to local port so we can receive pings
   si_this =
      {
      0
      };
   si_this.sin_family = AF_INET;
   si_this.sin_port = htons(32462);
   si_this.sin_addr.s_addr = htonl(INADDR_ANY);
   if (bind(broadcast, reinterpret_cast<sockaddr*>(&si_this), sizeof(si_this)) == -1)
      {
         handle_socket_error("binding broadcast socket", errno);
         return;
      }

   Discovery disc_msg;
   char uuid_buf[APR_UUID_FORMATTED_LENGTH];

   apr_uuid_t uuid;
   apr_uuid_get(&uuid);
   apr_uuid_format(uuid_buf, &uuid);
   disc_msg.set_node(std::string(uuid_buf, sizeof(uuid_buf)));

   disc_msg.SerializeToString(&broadcast_msg);
}

discovery::~discovery()
{
   //close(broadcast);
   //apr_pool_destroy(pool);
}

void discovery::announce()
{
   struct sockaddr_in si_that = si_this;
   inet_pton (AF_INET, "255.255.255.255", &si_that.sin_addr);
   if (sendto (broadcast, broadcast_msg.c_str(), broadcast_msg.size(), 
               0, reinterpret_cast<sockaddr*>(&si_that), sizeof (struct sockaddr_in)) == -1)
		{
			handle_socket_error("broadcasting presence", errno);
			return;
		}

   LOG4CXX_INFO(logger,
         "Sent present announcement (" << broadcast_msg.size() << " bytes)");
}

bool discovery::listen()
{
   // Extremely small timeout so that
	// we don't waste time waiting for announcements.
	struct timeval tv { 0, 1 };

   fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(broadcast, &read_fds);


	while(true)
		{
	// Check to see if the socket is ready to read.
   if(select(broadcast+1, &read_fds, nullptr, nullptr, &tv)==-1)
		{
			handle_socket_error("polling broadcast socket", errno);
			return false;
		}

	// See if the socket was selected.
   if (!FD_ISSET(broadcast, &read_fds))
		{
			return false;
		}

	// Read from the broadcast socket.
	char buffer[1024];
	struct sockaddr_in si_that;
	socklen_t si_len;
	ssize_t size = recvfrom (broadcast, buffer, 
                            sizeof(buffer), 0, 
                            reinterpret_cast<sockaddr*>(&si_that), 
                            &si_len);
	if (size == -1)
		{
			handle_socket_error("receiving broadcast", errno);
			return false;
		}

   // Try to parse a discovery message from the buffer.
	Discovery msg;
	if (!msg.ParseFromString(std::string(buffer, size)))
		{
			return false;
		}

   // Store the IP address and the last time
   // we heard from the node. This is used
	// to figure out how to route data requests
   node_map.insert(
      std::make_pair(
         msg.node(), 
         node_presence_type{si_that, std::chrono::steady_clock::now() }
      )
   );

   char addr_str[INET_ADDRSTRLEN];   
   LOG4CXX_INFO(logger,
					 "Received present announcement from " 
                << msg.node()
                << "("
                << inet_ntop(AF_INET, &(si_that.sin_addr), addr_str, sizeof(addr_str))
                << ":" << ntohs (si_that.sin_port) << ")"
   );
		}

   return true;
}

} // end namespace edge
} // end namespace lattice
