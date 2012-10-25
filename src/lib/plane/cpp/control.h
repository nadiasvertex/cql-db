/**
 * The control  plane, which provides a publish/subscribe 
 * model. Every object in the lattice connects to the control 
 * plane. The control planes for all groups in a lattice are also
 * connected together. 
 *
 */

#ifndef __LATTICE_CONTROL_PLANE_H__
#define __LATTICE_CONTROL_PLANE_H__

#include <memory>
#include <string>
#include <sstream>

#include <zmq.hpp>

#include <edge/proto/packet.pb.h>

namespace lattice {
  namespace plane {
    class control {
      zmq::socket_t publisher;

    public:
    control(zmq::context_t &ctx, int port):publisher(ctx, ZMQ_PUB) {

	std::string pgm_address = 
	  std::string("pgm://*:") +
	  std::to_string(port);

	publisher.bind("inproc://control");
	publisher.bind("ipc://lattice-control-plane");
	//publisher.bind(pgm_address.c_str());
	
      }

     void shutdown() {
    	 publisher.close();
     }

     void send(lattice::edge::Packet packet) {
    	 std::ostringstream out;
    	 packet.SerializeToOstream(&out);

    	 auto data = out.str();
    	 publisher.send(data.c_str(), data.size());
     }

    };

    typedef std::unique_ptr<control> control_t;

    /// Initialize the control plane.
    bool initialize_control(zmq::context_t& ctx);

    /// Shutdown the control plane.
    bool shutdown_control();

    /// Get access to the control plane.
    control_t& get_control();

  } // end namespace plane
} // end namespace control


#endif //__LATTICE_CONTROL_PLANE_H__
