/*
 * main.cpp
 *
 *  Created on: Oct 26, 2012
 *      Author: Christopher Nelson
 */

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include <plane/cpp/control.h>
#include "manager.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr logger(Logger::getLogger("cql.group"));

/**
 * Ensures that the control plane is shutdown on exit.
 */
class control_guard
{
public:
   control_guard(zmq::context_t &ctx)
   {
      lattice::plane::control::initialize(ctx);
   }

   ~control_guard()
   {
      lattice::plane::control::shutdown();
   }
};

int main(int argc, char*argv[])
{

   // Initialize the logger.
   BasicConfigurator::configure();

   LOG4CXX_DEBUG(logger, "initializing network transport");

   // Initialize 0mq.
   zmq::context_t ctx(1);

   LOG4CXX_DEBUG(logger, "initializing control plane");

   // Initialize control plane for local group.
   control_guard cg(ctx);

   LOG4CXX_DEBUG(logger, "initializing compute cell manager");

   // Setup the cell manager
   lattice::group::manager cm(ctx);

   LOG4CXX_INFO(logger, "Starting compute cells");

   // Start the cell manager
   //cm.start();

   LOG4CXX_INFO(logger, "Entering process loop");

   // Process requests
   cm.process();

   LOG4CXX_INFO(logger, "Shutdown");
}

