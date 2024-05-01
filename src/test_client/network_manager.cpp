// client
#include "network_manager.hpp"
#include "reader.hpp"
#include "share.hpp"
#include "writer.hpp"

// common
#include "../common/overload.hpp"
#include "../common/serial.hpp"

// unix (hopefully)
#include <netinet/in.h>
#include <poll.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

// std
#include <variant>

// spdlog
#include <spdlog/spdlog.h>

namespace test_client {

NetworkManager::NetworkManager(uint32_t ipv4, uint16_t port)
    : m_ipv4(ipv4), m_port(port)
{
}

NetworkManager::~NetworkManager()
{
    close();
}

void NetworkManager::close()
{
    if (share::reader_thread.is_initialized()
        && !share::reader_thread.has_joined()) {
        share::reader_thread.join();
    }

    if (share::writer_thread.is_initialized()
        && !share::writer_thread.has_joined()) {
        share::writer_thread.join();
    }
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
        spdlog::error(error.what());

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
        spdlog::error("writing failed, reason {}", write_status.what());

        return false;
    }

    auto [res, read_status] = m_channel.read(60000);

    if (read_status != ChannelErrorCode::OK) {
        spdlog::error("reading failed, reason {}", read_status.what());

        return false;
    }

    auto [payload, status] = deserialize<Payload>(res);

    if (status != DeserializeErrorCode::OK) {
        spdlog::error("reading failed, reason {}", read_status.what());

        return false;
    }

    if (std::holds_alternative<Decline>(payload)) {
        spdlog::warn(
            "connection declined, reason {}",
            std::get<Decline>(payload).reason
        );

        return false;
    }

    if (std::holds_alternative<Accept>(payload)) {
        return true;
    }

    spdlog::error("unexpected payload type {}", var_type(payload).name());

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

} // namespace test_client
