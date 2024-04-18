#pragma once

// common
#include <log.hpp>

// util
#include <utils.hpp>

// prot
#include <protocol.hpp>

namespace client {

class writer_t {
public:
    explicit writer_t(int m_conn_fd);

    void operator()();

    util::byte_vector get_message(
        prot::tagged_command_t& tagged_command);

private:
    // connection
    const int m_conn_fd;
};

} // namespace client
