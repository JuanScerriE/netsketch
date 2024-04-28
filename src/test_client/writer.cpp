// client
#include "writer.hpp"
#include "share.hpp"

// common
#include "../common/serial.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

// spdlog
#include <spdlog/spdlog.h>

namespace client {

Writer::Writer(Channel channel)
    : m_channel(channel)
{
}

void Writer::operator()()
{
    for (;;) {
        Action action {};

        {
            threading::unique_mutex_guard guard { share::writer_mutex };

            share::writer_cond.wait(guard, []() {
                return share::writer_queue.empty();
            });

            action = share::writer_queue.back();

            share::writer_queue.pop();
        }

        ByteString bytes { serialize<Payload>(TaggedAction { share::username,
                                                             action }) };

        auto status = m_channel.write(bytes);

        if (status != ChannelErrorCode::OK) {
            spdlog::error("writing failed, reason {}", status.what());

            break;
        }
    }

    shutdown();
}

void Writer::shutdown()
{
    if (share::reader_thread.is_initialized()
        && share::reader_thread.is_alive())
        share::reader_thread.cancel();
}

} // namespace client
