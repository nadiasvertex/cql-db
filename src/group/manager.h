/*
 * cell_manager.h
 *
 *  Created on: Nov 20, 2012
 *      Author: Christopher Nelson
 */

#ifndef __LATTICE_GROUP_CELL_MANAGER_H__
#define __LATTICE_GROUP_CELL_MANAGER_H__

#include <cstdint>
#include <thread>
#include <vector>
#include <zmq.hpp>

#include <cell/cpp/command_processor.h>

namespace lattice {
namespace group {

class manager
{
   zmq::context_t &ctx;
   std::vector<cell::command_processor*> processors;
   std::vector<std::thread*> threads;

   bool continue_processing;

private:
   /** Issues the 'stop' command when sig_term is called. */
   static void sig_term_handler(int ignored);

   /** Handles spawning new threads to deal with query execution
    * requests. */
   void query_processor_thread();

public:
   manager(zmq::context_t &_ctx) :
         ctx(_ctx), continue_processing(true)
   {
   }

   ~manager();

   /**
    * Starts the cell manager.
    */
   void start();

   /**
    * Issues a stop request and waits for all threads to join.
    */
   void stop();

   /**
    * Enters a loop waiting for any of the cells to process data. Stops
    * when stop() is called, or SIGTERM is sent.
    */
   void process();
};

} // end namespace lattice
} // end namespace group

#endif /* __LATTICE_GROUP_CELL_MANAGER_H__ */
