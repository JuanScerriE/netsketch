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

    void handle_payload(util::ByteVector& payload);

    void update_list(prot::TaggedCommand& tagged_command);
    void update_whole_list(prot::TaggedDrawList& list);

    const int m_conn_fd;

    // logging
    static logging::log log;

    static void setup_logging();
};

} // namespace client
