#include "runner.hpp"

// std
#include <variant>

// common
#include "../common/overload.hpp"
#include "../common/types.hpp"

// cstd
#include <cstdlib>

// unix
#include <arpa/inet.h>
#include <netinet/in.h>

// fmt
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>

// raylib
#include <raylib.h>

namespace exporter {

bool Runner::setup(
    const std::string& username,
    const std::string& ipv4_addr,
    uint16_t port
)
{
    m_username = username;

    // setup network info

    struct in_addr addr { };

    if (inet_pton(AF_INET, ipv4_addr.c_str(), &addr) <= 0) {
        fmt::println(stderr, "error: invalid IPv4 address");

        return false;
    }

    // NOTE: this is in host-readable form
    m_ipv4_addr = ntohl(addr.s_addr);

    m_port = port;

    // setup socket

    try {
        m_sock.open(SOCK_STREAM, 0);

        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(m_ipv4_addr);
        addr.sin_port = htons(m_port);

        m_sock.connect(&addr);
    } catch (std::runtime_error& error) {
        fmt::println(
            stderr,
            "error: opening & connecting failed, reason {}",
            error.what()
        );

        return false;
    }

    m_channel = Channel { m_sock };

    // check username

    ByteString req { serialize<Payload>(Username { username }) };

    auto write_status = m_channel.write(req);

    if (write_status != ChannelErrorCode::OK) {
        fmt::println(
            stderr,
            "error: writing failed, reason {}",
            write_status.what()
        );

        return false;
    }

    auto [res, read_status] = m_channel.read(60000); // wait for a minute

    if (read_status != ChannelErrorCode::OK) {
        fmt::println(
            stderr,
            "error: reading failed, reason {}",
            read_status.what()
        );

        return false;
    }

    auto [payload, status] = deserialize<Payload>(res);

    if (status != DeserializeErrorCode::OK) {
        fmt::println(
            stderr,
            "error: deserialization failed, reason {}",
            status.what()
        );

        return false;
    }

    if (std::holds_alternative<Decline>(payload)) {
        fmt::println(
            stderr,
            "error: connection declined, reason {}",
            std::get<Decline>(payload).reason
        );

        return false;
    }

    if (!std::holds_alternative<Accept>(payload)) {
        fmt::println(
            stderr,
            "error: unexpected payload type {}",
            var_type(payload).name()
        );

        return false;
    }

    return true;
}

[[nodiscard]] bool Runner::run()
{
    // read full list

    auto [bytes, read_status] = m_channel.read();

    if (read_status != ChannelErrorCode::OK) {
        fmt::println(
            stderr,
            "error: reading failed, reason {}",
            read_status.what()
        );

        return false;
    }

    // deserialize

    auto [payload, deser_status] = deserialize<Payload>(bytes);

    if (deser_status != DeserializeErrorCode::OK) {
        fmt::println(
            stderr,
            "error: deserialization failed, reason {}",
            deser_status.what()
        );

        return false;
    }

    // convert to list

    if (!std::holds_alternative<TaggedDrawVector>(payload)) {
        fmt::println(
            stderr,
            "error: unexpected type {}",
            var_type(payload).name()
        );

        return false;
    }

    TaggedDrawVector draws = std::get<TaggedDrawVector>(payload);

    // generate image
    if (!generate_image(draws)) {
        fmt::println(stderr, "error: failed to generate image");

        return false;
    }

    return true;
}

[[nodiscard]] Color to_raylib_colour(Colour colour)
{
    return { colour.r, colour.g, colour.b, 255 };
}

void process_draw(Image* image, Draw& draw)
{
    std::visit(
        overload {
            [/*image*/](TextDraw& arg) {
                (void)arg;
                // ImageDrawText(
                //     image,
                //     arg.string.c_str(),
                //     arg.x,
                //     arg.y,
                //     20, // NOTE: maybe change this?
                //     to_raylib_colour(arg.colour)
                // );
            },
            [image](CircleDraw& arg) {
                ImageDrawCircle(
                    image,
                    arg.x,
                    arg.y,
                    static_cast<int>(arg.r),
                    to_raylib_colour(arg.colour)
                );
            },
            [image](RectangleDraw& arg) {
                ImageDrawRectangle(
                    image,
                    arg.x0,
                    arg.y0,
                    arg.x1 - arg.x0,
                    arg.y1 - arg.y0,
                    to_raylib_colour(arg.colour)
                );
            },
            [image](LineDraw& arg) {
                ImageDrawLine(
                    image,
                    arg.x0,
                    arg.y0,
                    arg.x1,
                    arg.y1,
                    to_raylib_colour(arg.colour)
                );
            },
        },
        draw
    );
}

bool Runner::generate_image(TaggedDrawVector& draws)
{
    Color teal95 = { 0, 128, 128, 255 };

    Image image = GenImageColor(3024, 1964, teal95);

    for (auto& tagged_draw : draws) {
        process_draw(&image, tagged_draw.draw);
    }

    if (!ExportImage(image, "image.png")) {
        return false;
    }

    return true;
}

Runner::~Runner()
{
}

} // namespace client
