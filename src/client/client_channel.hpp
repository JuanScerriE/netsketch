#pragma once

// client
#include "../common/overload.hpp"
#include "../common/tagged_draw_vector_wrapper.hpp"
#include "../network/channel.hpp"
#include "../threading/threading.hpp"
#include "draw_list.hpp"
#include "serial.hpp"
#include "share.hpp"

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
                    threading::mutex_guard guard { share::writer_mutex };

                    {
                        threading::rwlock_wrguard wrguard { share::rwlock1 };

                        TaggedDrawVectorWrapper { share::vec1 }.adopt(
                            arg.username
                        );
                    }

                    {
                        threading::rwlock_wrguard wrguard { share::rwlock2 };

                        TaggedDrawVectorWrapper { share::vec1 }.adopt(
                            arg.username
                        );
                    }
                },
                [](Decline& arg) {
                },
                [](Accept&) {
                    log.info("connection accepted");
                },
                [](Username&) {
                    ABORT("unexpected type");
                },
                [](TaggedDrawVector& arg) {
                    threading::mutex_guard guard { share::writer_mutex };

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
                    threading::mutex_guard guard { share::writer_mutex };

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

        if (std::holds_alternative<prot::TaggedCommand>(payload_object)) {
            auto& tagged_command
                = std::get<prot::TaggedCommand>(payload_object);
            update_list(tagged_command);

            return;
        }

        if (std::holds_alternative<prot::TaggedDraw>(payload_object)) {
            auto& tagged_draw = std::get<prot::TaggedDraw>(payload_object);
            (void)tagged_draw;

            return;
        }

        throw std::runtime_error("does not contain known type");
    }

    void send(std::any object) override
    {
        (void)object;
    }

    void shutdown() override
    {
    }
};

} // namespace client
