#include <memory>
#include <plane/cpp/control.h>

namespace lattice {
namespace plane {

static control_t _control_plane_singleton;

control::control(zmq::context_t &ctx, int port) :
      publisher(ctx, ZMQ_PUB), response(ctx, ZMQ_REP)
{

   std::string pgm_address = std::string("epgm://*:") + std::to_string(port);

   // Broadcast to the inproc control channel and on IPC if
   // anyone is listening.
   publisher.bind("inproc://control");
   publisher.bind("ipc://lattice-control-plane");

   // Listen for requests to broadcast locally.
   response.bind("inproc://control-post");

   // Spawn a thread to proxy messages from the post
   // channel to the broadcast channel.
   fwd_thread = std::unique_ptr<std::thread>(new std::thread([&]
      {
         zmq_proxy(response, publisher, nullptr);
      }));

}

void control::_shutdown()
{
   publisher.close();
   response.close();
   fwd_thread->join();
}

bool control::initialize(zmq::context_t& ctx)
{
   _control_plane_singleton = control_t(new lattice::plane::control(ctx, 9999));
   return true;
}

bool control::shutdown()
{
   _control_plane_singleton->_shutdown();
   return true;
}

void control::send(const lattice::edge::Packet& packet)
{
   std::ostringstream out;
   packet.SerializeToOstream(&out);

   auto data = out.str();
   publisher.send(data.c_str(), data.size());
}

control_t& control::get()
{
   return _control_plane_singleton;
}

}
}

