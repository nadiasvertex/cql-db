#include <ctime>
#include <sstream>

#include <plane/cpp/control.h>
#include <edge/proto/packet.pb.h>

#include <gtest/gtest.h>

static timespec delay = {0, 100000000};

class control_guard {
public:
	control_guard(zmq::context_t &ctx) {
		lattice::plane::control::initialize(ctx);
	}

	~control_guard() {
		lattice::plane::control::shutdown();
	}
};

TEST(ControlPlaneTest, CanInitialize) {
  zmq::context_t ctx(1);

  ASSERT_NO_THROW(lattice::plane::control::initialize(ctx));
  ASSERT_NO_THROW(lattice::plane::control::shutdown());
}

TEST(ControlPlaneTest, CanConnect) {
  zmq::context_t ctx(1);

  zmq::socket_t sub(ctx, ZMQ_SUB);

  control_guard cg(ctx);

  ASSERT_NO_THROW(sub.connect("inproc://control"));
}

TEST(ControlPlaneTest, CanBroadcast) {
  zmq::context_t ctx(1);
  zmq::socket_t sub(ctx, ZMQ_SUB);

  lattice::edge::Packet p1;

  p1.Clear();
  p1.mutable_src()->set_unit(0);
  p1.add_dst()->set_unit(1);

  control_guard cg(ctx);

  sub.connect("inproc://control");

  ASSERT_NO_THROW(lattice::plane::control::get()->send(p1));
}

/*
TEST(ControlPlaneTest, CanReceiveBroadcast) {
  zmq::context_t ctx(1);
  zmq::socket_t sub(ctx, ZMQ_SUB);

  lattice::edge::Packet p1, p2;

  p1.Clear();
  p1.mutable_src()->set_cell(0);
  p1.add_dst()->set_cell(1);

  control_guard cg(ctx);

  sub.connect("inproc://control");
  sub.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);


  ::nanosleep(&delay, nullptr);

  lattice::plane::control::get()->send(p1);

  ::nanosleep(&delay, nullptr);

  zmq::message_t msg;
  sub.recv(&msg);

  std::string data(static_cast<char*>(msg.data()), msg.size());
  std::istringstream in(data);

  ASSERT_NO_THROW(p2.ParseFromIstream(&in));

  EXPECT_EQ(p2.src().cell(), p1.src().cell());
  EXPECT_EQ(p2.dst_size(), p1.dst_size());
  EXPECT_EQ(p2.dst(0).cell(), p1.dst(0).cell());
}

TEST(ControlPlaneTest, CanForwardBroadcast) {
  zmq::context_t ctx(1);
  zmq::socket_t sub(ctx, ZMQ_SUB);
  zmq::socket_t req(ctx, ZMQ_REQ);

  lattice::edge::Packet p1, p2;

  p1.Clear();
  p1.mutable_src()->set_cell(0);
  p1.add_dst()->set_cell(1);

  control_guard cg(ctx);

  sub.connect("inproc://control");
  sub.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);

  req.connect("inproc://control-post");

  ::nanosleep(&delay, nullptr);

  {
      std::ostringstream out;
      p1.SerializeToOstream(&out);

      auto data = out.str();
      req.send(data.c_str(), data.size());
  }

  ::nanosleep(&delay, nullptr);

  zmq::message_t msg;
  sub.recv(&msg);

  std::string data(static_cast<char*>(msg.data()), msg.size());
  std::istringstream in(data);

  ASSERT_NO_THROW(p2.ParseFromIstream(&in));

  EXPECT_EQ(p2.src().cell(), p1.src().cell());
  EXPECT_EQ(p2.dst_size(), p1.dst_size());
  EXPECT_EQ(p2.dst(0).cell(), p1.dst(0).cell());
}
*/
