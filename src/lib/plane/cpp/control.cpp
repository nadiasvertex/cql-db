#include <memory>
#include <plane/cpp/control.h>

namespace lattice {
namespace plane {

static control_t _control_plane_singleton;

bool control::initialize(zmq::context_t& ctx) {
	_control_plane_singleton = control_t(
			new lattice::plane::control(ctx, 9999));
	return true;
}

bool control::shutdown() {
	_control_plane_singleton->_shutdown();
	return true;
}

control_t& control::get() {
	return _control_plane_singleton;
}

}
}

