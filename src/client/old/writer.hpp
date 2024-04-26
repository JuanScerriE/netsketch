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

    void dtor();

   private:
    // connection
    const int m_conn_fd;

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace client
