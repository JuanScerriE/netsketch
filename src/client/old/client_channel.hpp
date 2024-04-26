#pragma once

// client
#include "../common/overload.hpp"
#include "../common/tagged_draw_vector_wrapper.hpp"
#include "../network/channel.hpp"
#include "../threading/threading.hpp"
#include "serial.hpp"
#include "share.hpp"

// std
#include <any>

namespace client {

class ClientChannel : public Channel {
   public:
    explicit ClientChannel(IPv4Socket& conn_sock)
        : Channel(conn_sock)
    {
    }

    void handle(ByteVector bytes) override
    {
        Deserialize deserializer { bytes };

        Payload payload = deserializer.payload();

        std::visit(
            overload {
                [](Adopt& arg) {
                    threading::mutex_guard guard {
                        share::tagged_draw_vector_mutex
                    };

                    {
                        threading::rwlock_wrguard wrguard { share::rwlock1 };

                        TaggedDrawVectorWrapper { share::vec1 }.adopt(arg);
                    }

                    {
                        threading::rwlock_wrguard wrguard { share::rwlock2 };

                        TaggedDrawVectorWrapper { share::vec1 }.adopt(arg);
                    }
                },
                [](Decline& arg) {
                    (void)arg;
                },
                [](Accept&) {
                    log.info("connection accepted");
                },
                [](Username&) {
                    ABORT("unexpected type");
                },
                [](TaggedDrawVector& arg) {
                    threading::mutex_guard guard {
                        share::tagged_draw_vector_mutex
                    };

                    {
                        threading::rwlock_wrguard wrguard { share::rwlock1 };

                        share::vec1 = arg;
                    }

                    {
                        threading::rwlock_wrguard wrguard { share::rwlock2 };

                        share::vec2 = arg;
                    }
                },
                [](TaggedDraw&) {
                    ABORT("unexpected type");
                },
                [](TaggedAction& arg) {
                    threading::mutex_guard guard {
                        share::tagged_draw_vector_mutex
                    };

                    {
                        threading::rwlock_wrguard wrguard { share::rwlock1 };

                        TaggedDrawVectorWrapper { share::vec1 }.update(arg);
                    }

                    {
                        threading::rwlock_wrguard wrguard { share::rwlock2 };

                        TaggedDrawVectorWrapper { share::vec1 }.update(arg);
                    }
                },
            },
            payload
        );
    }

    void send(std::any object) override
    {
        if (object.type() == typeid(Username)) {
            send(std::any_cast<Username>(object));
        }

        if (object.type() == typeid(TaggedAction)) {
            send(std::any_cast<TaggedAction>(object));
        }

        ABORT("unreachable");
    }

    void send(const Username& username)
    {
        Serialize serialize { username };

        {
            std::scoped_lock lock { m_mutex };

            m_writer_queue.push(serialize.bytes());
        }

        m_cond_var.notify_one();
    }

    void send(const TaggedAction& tagged_action)
    {
        Serialize serialize { tagged_action };

        {
            std::scoped_lock lock { m_mutex };

            m_writer_queue.push(serialize.bytes());
        }

        m_cond_var.notify_one();
    }

    void shutdown() override
    {
    }
};

} // namespace client
