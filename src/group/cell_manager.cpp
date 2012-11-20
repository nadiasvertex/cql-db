#include <signal.h>
#include <chrono>
#include <mutex>

#include <edge/proto/packet.pb.h>
#include "cell_manager.h"

namespace lattice {
namespace group {

/** Make sure that only one stop attempt at a time proceeds. */
static std::mutex stop_guard;

/** The one and only cell_manager. */
static cell_manager* _manager = nullptr;

void cell_manager::sig_term_handler(int ignored)
{
   if (_manager != nullptr)
      {
         _manager->stop();
      }
}

void cell_manager::start()
{
   _manager = this;

   signal((int) SIGINT, cell_manager::sig_term_handler);

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
}

void cell_manager::stop()
{
   std::lock_guard<std::mutex> lock(stop_guard);

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
   src->set_cell(0);

   auto out = packet.SerializeAsString();
   request.send(out.c_str(), out.size());

   for (auto t : threads)
      {
         t->join();
      }
}

void cell_manager::process()
{
   std::chrono::milliseconds dur(1000);

   while (continue_processing)
      {
         std::this_thread::sleep_for(dur);
      }
}

cell_manager::~cell_manager()
{
   if (continue_processing)
      {
         stop();
      }
}

} // end namespace lattice
} // end namespace group

