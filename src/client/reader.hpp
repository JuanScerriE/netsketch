#pragma once

namespace client {

class reader_t {
public:
    explicit reader_t(int conn_fd);

    void operator()();

private:
    void handle_loop();

    const int m_conn_fd;
};

} // namespace client
