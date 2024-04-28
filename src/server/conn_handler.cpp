// server
#include "conn_handler.hpp"
#include "share.hpp"
#include "timing.hpp"

// unix
#include <arpa/inet.h>
#include <fmt/format.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <unistd.h>

// utils
#include <variant>

// common
#include "../common/abort.hpp"
#include "../common/tagged_draw_vector_wrapper.hpp"
#include "../common/threading.hpp"

#define MINUTE (60000)

namespace server {

ConnHandler::ConnHandler(IPv4SocketRef sock, std::string username)
    : m_sock(sock), m_channel(m_sock), m_username(std::move(username))
{
}

void ConnHandler::setup_readable_net_info()
{
    auto addr = m_sock.get_sockaddr_in();

    char ipv4[INET_ADDRSTRLEN + 1];

    if (inet_ntop(AF_INET, &addr.sin_addr, ipv4, INET_ADDRSTRLEN) == nullptr) {
        ABORTV("inet_ntop(...) failed, reason: {}", strerror(errno));
    }

    m_ipv4 = std::string { ipv4 };

    m_port = std::to_string(ntohs(addr.sin_port));
}

void ConnHandler::operator()()
{
    setup_readable_net_info();

    setup_logging(m_ipv4, m_port);

    log.info("received a connection from {}:{}", m_ipv4, m_port);

    pthread_cleanup_push(
        [](void* untyped_self) {
            auto* self = static_cast<ConnHandler*>(untyped_self);

            create_client_timer(self->m_username);

            {
                threading::mutex_guard guard { share::connections_mutex };

                share::connections.erase(self->m_sock.native_handle());
            }

            {
                threading::mutex_guard guard { share::users_mutex };

                share::users.erase(self->m_username);
            }
        },
        this
    );

    if (!send_full_list()) {
        log.info("failed to send full list");

        return;
    }

    for (;;) {
        auto [res, status] = m_channel.read(MINUTE * 10);

        if (status != ChannelErrorCode::OK) {
            log.info("reading failed, reason: {}", status.what());

            break;
        }

        log.debug("payload size {} bytes", res.size());

        handle_payload(res);

        log.flush();
    }

    log.info("closing connection handler");

    pthread_cleanup_pop(1);
}

bool ConnHandler::send_full_list()
{
    TaggedDrawVector tagged_draw_vector {};

    {
        threading::unique_mutex_guard guard { share::update_mutex };

        tagged_draw_vector = share::tagged_draw_vector;
    }

    auto status = m_channel.write(serialize<Payload>(tagged_draw_vector));

    if (status != ChannelErrorCode::OK) {
        log.info("writing failed, reason: {}", status.what());

        return false;
    }

    return true;
}

void ConnHandler::handle_payload(const ByteString& bytes)
{
    auto [payload, status] = deserialize<Payload>(bytes);

    if (status != DeserializeErrorCode::OK) {
        log.warn("deserialization failed, reason {}", status.what());
        return;
    }

    if (!std::holds_alternative<TaggedAction>(payload)) {
        log.warn("unexpected payload type {}", var_type(payload).name());

        return;
    }

    TaggedAction tagged_action = std::get<TaggedAction>(payload);

    // NOTE: if we block on the push queue we might actually
    // miss data or overflow data that is sent to us in
    // buffer. That's what we are doing right now

    {
        threading::unique_mutex_guard guard { share::update_mutex };

        TaggedDrawVectorWrapper { share::tagged_draw_vector }.update(
            tagged_action
        );

        share::payload_queue.emplace(tagged_action);
    }

    share::update_cond.notify_one();
}

logging::log ConnHandler::log {};

void ConnHandler::setup_logging(std::string ipv4, std::string port)
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix(fmt::format("[{}:{}]", ipv4, port));
}

} // namespace server
