#include <memory>
#include <plane/control.h>

static std::unique_ptr<lattice::plane::control> _control_plane_singleton;

bool initialize_control_plane(zmq::context_t& ctx) {
  _control_plane_singleton = new lattice::plane::control(ctx, 9999);
}

