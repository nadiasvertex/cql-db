#include <chrono>
#include <mutex>

#include <apr-1/apr_signal.h>
#include <log4cxx/logger.h>

#include <edge/proto/packet.pb.h>
#include <processor/cpp/query.h>
#include "manager.h"

using namespace log4cxx;

extern LoggerPtr logger;

namespace lattice {
namespace group {

/** Make sure that only one stop attempt at a time proceeds. */
static std::mutex stop_guard;

/** The one and only cell_manager. */
static manager* _manager = nullptr;

void manager::sig_term_handler(int ignored)
{
   LOG4CXX_DEBUG(logger, "interrupt or termination signal received");

   if (_manager != nullptr)
      {
         _manager->stop();
      }
}

void manager::start()
{
   _manager = this;

   LOG4CXX_DEBUG(logger, "cell manager started, using "
                         << std::thread::hardware_concurrency()
                         << " hardware threads.");

   apr_signal((int) SIGINT, manager::sig_term_handler);
   apr_signal((int) SIGTERM, manager::sig_term_handler);

   // Start the cell processors.
   for (auto i = 0; i < std::thread::hardware_concurrency(); ++i)
      {
         auto* processor = new cell::command_processor();
         processors.push_back(processor);

         // Create a new thread.
         threads.push_back(
               new std::thread([&]
                  {
                     auto* cp = processor;
                     zmq::socket_t control(ctx, ZMQ_SUB);

                     // Subscribe to the control channel.
                     control.connect("inproc://control");
                     control.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);

                     while(continue_processing)
                        {
                           zmq::message_t msg;
                           control.recv(&msg);

                           std::string data(static_cast<char*>(msg.data()), msg.size());
                           std::istringstream in(data);

                           // A generic data packet.
                           edge::Packet packet;
                           packet.ParseFromIstream(&in);
                        }
                  }));
      }

   threads.push_back(new std::thread([&]
      {
         query_processor_thread();
      }));

   LOG4CXX_DEBUG(logger, "cell manager using "
                            << threads.size()
                            << " threads.");
}

void manager::stop()
{
   std::lock_guard<std::mutex> lock(stop_guard);

   LOG4CXX_DEBUG(logger, "cell manager stop requested.");

   if (continue_processing == false)
      {
         return;
      }

   _manager = nullptr;
   continue_processing = false;

   zmq::socket_t request(ctx, ZMQ_REQ);

   // Send a stop packet to the cells.
   request.connect("inproc://control-post");
   edge::Packet packet;

   packet.set_type(edge::Packet::STOP);

   auto* src = packet.mutable_src();
   src->set_lattice(0);
   src->set_group(0);
   src->set_unit(0);

   LOG4CXX_DEBUG(logger, "sending STOP command to compute cells");
   auto out = packet.SerializeAsString();
   request.send(out.c_str(), out.size());

   for (auto t : threads)
      {
         LOG4CXX_DEBUG(logger, "joining thread " << t->get_id());
         t->join();
      }

   LOG4CXX_DEBUG(logger, "cell manager stop completed.");
}

void manager::process()
{
   std::chrono::milliseconds dur(1000);

   LOG4CXX_DEBUG(logger, "enter cell manager process loop.");

   while (continue_processing)
      {
         std::this_thread::sleep_for(dur);
      }

   LOG4CXX_DEBUG(logger, "exit cell manager process loop.");
}

manager::~manager()
{
   if (continue_processing)
      {
         stop();
      }
}

} // end namespace lattice
} // end namespace group

