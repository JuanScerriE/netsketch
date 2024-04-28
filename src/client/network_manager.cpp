// client
#include "network_manager.hpp"
#include "reader.hpp"
#include "share.hpp"
#include "writer.hpp"

// common
#include "../common/log.hpp"
#include "../common/serial.hpp"
#include "../common/overload.hpp"

// unix (hopefully)
#include <netinet/in.h>
#include <poll.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

// std
#include <variant>

namespace client {

NetworkManager::NetworkManager(uint32_t ipv4, uint16_t port)
    : m_ipv4(ipv4), m_port(port)
{
    setup_logging();
}

NetworkManager::~NetworkManager()
{
    close_reader();
    close_writer();
}

bool NetworkManager::setup()
{
    if (!open_socket()) {
        return false;
    }

    wrap_socket();

    if (!send_username()) {
        return false;
    }

    start_reader();
    start_writer();

    return true;
}

bool NetworkManager::open_socket()
{
    try {
        m_conn.open(SOCK_STREAM, 0);

        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(m_ipv4);
        addr.sin_port = htons(m_port);

        m_conn.connect(&addr);
    } catch (std::runtime_error& error) {
        log.error(error.what());

        return false;
    }

    return true;
}

void NetworkManager::wrap_socket()
{
    m_channel = Channel { m_conn };
}

bool NetworkManager::send_username()
{
    Username username { share::username };

    ByteString req { serialize<Payload>({ username }) };

    auto write_status = m_channel.write(req);

    if (write_status != ChannelErrorCode::OK) {
        log.error("writing failed, reason {}", write_status.what());

        return false;
    }

    auto [res, read_status] = m_channel.read(60000);

    if (read_status != ChannelErrorCode::OK) {
        log.error("reading failed, reason {}", read_status.what());

        return false;
    }

    auto [payload, status] = deserialize<Payload>(res);

    if (status != DeserializeErrorCode::OK) {
        log.error("reading failed, reason {}", read_status.what());

        return false;
    }

    if (std::holds_alternative<Decline>(payload)) {
        log.warn(
            "connection declined, reason {}",
            std::get<Decline>(payload).reason
        );

        return false;
    }

    if (std::holds_alternative<Accept>(payload)) {
        return true;
    }

    log.error("unexpected payload type {}", var_type(payload).name());

    return false;
}

void NetworkManager::start_reader()
{
    share::reader_thread = threading::thread { Reader { m_channel } };
}

void NetworkManager::start_writer()
{
    share::writer_thread = threading::thread { Writer { m_channel } };
}

void NetworkManager::close_reader()
{
    if (share::reader_thread.is_initialized())
        share::reader_thread.join();
}

void NetworkManager::close_writer()
{
    if (share::writer_thread.is_initialized())
        share::writer_thread.join();
}

logging::log NetworkManager::log {};

void NetworkManager::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix("[network_manager]");

    log.set_file(share::log_file);
}

} // namespace client
