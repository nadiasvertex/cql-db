/*
 * main.cpp
 *
 *  Created on: Oct 26, 2012
 *      Author: Christopher Nelson
 */

#include <plane/cpp/control.h>
#include "manager.h"

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

    // Initialize 0mq.
    zmq::context_t ctx(1);

    // Initialize control plane for local group.
    control_guard cg(ctx);

    // Setup the cell manager
    lattice::group::manager cm(ctx);

    // Start the cell manager
    cm.start();

    // Process requests
    cm.process();
}



