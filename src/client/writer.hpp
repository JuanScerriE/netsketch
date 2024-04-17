#pragma once

// common
#include <log.hpp>
#include <serialization.hpp>

namespace client {

class writer_t {
public:
    explicit writer_t(int m_conn_fd);

    void operator()();

    common::message_t get_message();

private:
    // connection
    const int m_conn_fd;
};

} // namespace client
