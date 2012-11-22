#include <apr-1/apr_uuid.h>
#include <log4cxx/logger.h>

#include <edge/cpp/discovery.h>
#include <edge/proto/discovery.pb.h>

using namespace log4cxx;

static LoggerPtr logger(Logger::getLogger("cql.edge"));

namespace lattice {
namespace edge {

#define CHECK_ERROR                          \
	if (status != APR_SUCCESS) {             \
			handle_socket_error (status);    \
			return;                          \
    }

void discovery::handle_socket_error(apr_status_t status) {
	char buf[8192];
	LOG4CXX_ERROR(logger,
			"Discovery socket error: " << apr_strerror(status, buf, sizeof(buf)));
}

discovery::discovery(int port) {
	auto status = apr_pool_create(&pool, nullptr);
	CHECK_ERROR;

	status = apr_sockaddr_info_get(&broadcast_address, "255.255.255.255",
			APR_INET, port, 0, pool);
	CHECK_ERROR;

	status = apr_socket_create(&broadcast, APR_INET, SOCK_DGRAM, APR_PROTO_UDP,
			pool);
	CHECK_ERROR;

	apr_socket_opt_set(broadcast, APR_SO_NONBLOCK, 1);

	Discovery disc_msg;
	char uuid_buf[APR_UUID_FORMATTED_LENGTH];

	apr_uuid_t uuid;
	apr_uuid_get(&uuid);
	apr_uuid_format(uuid_buf, &uuid);
	disc_msg.set_node(std::string(uuid_buf, sizeof(uuid_buf)));

	disc_msg.SerializeToString(&broadcast_msg);
}

void discovery::announce() {
	apr_size_t buf_size = broadcast_msg.size();
	apr_socket_sendto(broadcast, broadcast_address, 0, broadcast_msg.c_str(),
			&buf_size);
}

} // end namespace edge
} // end namespace lattice
