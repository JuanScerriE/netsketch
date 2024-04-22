#pragma once

// logging
#include <log.hpp>

#include "protocol.hpp"
#include "utils.hpp"

namespace client {

class reader_t {
public:
    explicit reader_t(int conn_fd);

    void operator()();

    void shutdown();

    void dtor();

private:
    void handle_loop();

    void handle_payload(util::byte_vector& payload);

    void update_list(
        prot::tagged_command_t& tagged_command);
    void update_whole_list(prot::tagged_draw_list_t& list);

    const int m_conn_fd;

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace client
