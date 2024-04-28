// client
#include "writer.hpp"
#include "share.hpp"

// common
#include "../common/serial.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

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

        ByteString bytes { serialize<Payload>(TaggedAction { share::username, action }
        ) };

        // log.debug("sending: 0x{}", bytes_to_hex(bytes));

        auto status = m_channel.write(bytes);

        if (status != ChannelErrorCode::OK) {
            log.error("writing failed, reason {}", status.what());

            break;
        }

        log.flush();
    }

    shutdown();
}

void Writer::shutdown()
{
    if (share::reader_thread.is_initialized()
        && share::reader_thread.is_alive())
        share::reader_thread.cancel();
    if (share::input_thread.is_initialized() && share::input_thread.is_alive())
        share::input_thread.cancel();

    share::run_gui = false;
}

logging::log Writer::log {};

void Writer::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix("[writer]");

    log.set_file(share::log_file);
}

} // namespace client
