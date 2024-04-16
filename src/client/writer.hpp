#pragma once

#include <log.hpp>

namespace client {

class writer_t {
   public:
    explicit writer_t(
        int m_conn_fd
    );

    void operator()();

   private:
    void handle_loop();

    // connection
    const int m_conn_fd;
};

} // namespcae client
