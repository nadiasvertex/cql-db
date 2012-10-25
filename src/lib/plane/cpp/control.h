/**
 * The control  plane, which provides a publish/subscribe 
 * model. Every object in the lattice connects to the control 
 * plane. The control planes for all groups in a lattice are also
 * connected together. 
 *
 */

#ifndef __LATTICE_CONTROL_PLANE_H__
#define __LATTICE_CONTROL_PLANE_H__

#include <string>
#include <zmq.hpp>

#include <edge/proto/address.pb.h>


namespace lattice {
  namepace plane {
    class control {
      zmq::socket_t publisher;

    public:
    control(zmq::context_t &ctx, int port):publisher(ctx, ZMQ_PUB) {

	std::string pgm_address = 
	  std::string("pgm://*:") +
	  std::to_string(port);

	publisher.bind("inproc://control");
	publisher.bind("ipc://lattice-control-plane");
	publisher.bind(pgm_address);
	
      }
    };
  } // end namespace plane
} // end namespace control


#endif //__LATTICE_CONTROL_PLANE_H__
