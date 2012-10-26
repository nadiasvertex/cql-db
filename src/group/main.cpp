/*
 * main.cpp
 *
 *  Created on: Oct 26, 2012
 *      Author: Christopher Nelson
 */

#include <plane/cpp/control.h>

/**
 * Ensures that the control plane is shutdown on exit.
 */
class control_guard {
public:
    control_guard(zmq::context_t &ctx) {
        lattice::plane::control::initialize(ctx);
    }

    ~control_guard() {
        lattice::plane::control::shutdown();
    }
};

int main(int argc, char*argv[]) {

    // Initialize zero mq.
    zmq::context_t ctx(1);

    // Initialize control plane for local group.
    control_guard cg(ctx);

}



