#include <memory>
#include <plane/cpp/control.h>

namespace lattice {
namespace plane {

static control_t _control_plane_singleton;

bool initialize_control(zmq::context_t& ctx) {
	_control_plane_singleton = control_t(
			new lattice::plane::control(ctx, 9999));
	return true;
}

bool shutdown_control() {
	_control_plane_singleton->shutdown();
	return true;
}

control_t& get_control() {
	return _control_plane_singleton;
}

}
}

