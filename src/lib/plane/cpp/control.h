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
#include <thread>

#include <zmq.hpp>

#include <edge/proto/packet.pb.h>

namespace lattice
{
namespace plane
{

class control;
typedef std::unique_ptr<control> control_t;

class control
{
    zmq::socket_t publisher;
    zmq::socket_t response;

    std::unique_ptr<std::thread> fwd_thread;

    /** Initialize the control plane
     *
     * @param ctx: The zmq context.
     * @param port: The port to broadcast on for pgm and epgm broadcasts.
     *
     * Initializes and binds to all sockets.
     */
    control(zmq::context_t &ctx, int port);

    /** Shutdown the control plane.
     *
     * Close all control plane bindings. After this call
     * you can no longer send messages using the control plane.
     */
    void _shutdown();

public:
    ~control() {
        //_shutdown();
    }

    /** Send a message on the control plane.
     *
     * @param packet: The packet to broadcast on the control plane.
     */
    void send(const lattice::edge::Packet& packet);

    /** Initialize the control plane.
     *
     * @param ctx: The zmq context.
     *
     * Creates a new control plane singleton for this group. Once
     * initialized, the control plane can be fetched using get().
     */
    static bool initialize(zmq::context_t& ctx);

    /** Shutdown the active control plane.
     */
    static bool shutdown();

    /** Get access to the active control plane.
     *
     * Provides a handle to the active control plane. This handle must not
     * be stored in any way.
     */
    static control_t& get();

};

} // end namespace plane
} // end namespace control

#endif //__LATTICE_CONTROL_PLANE_H__
