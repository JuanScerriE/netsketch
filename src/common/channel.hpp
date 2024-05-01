#pragma once

// std
#include <optional>

// arpa
#include <arpa/inet.h>

// common
#include "network.hpp"
#include "serial.hpp"

#define MAGIC_BYTES (static_cast<short>(0x2003))

struct Header {
    std::uint16_t magic_bytes;
    std::size_t payload_size;

    [[nodiscard]] constexpr static std::size_t size()
    {
        return sizeof(decltype(Header::magic_bytes))
               + sizeof(decltype(Header::payload_size))
               + 1; // to record the endianness (used cereal)
    }

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(magic_bytes, payload_size);
    }
};

enum class ChannelErrorCode {
    END_OF_FILE,
    ERRNO,
    HUNG_UP,
    DESERIALIZATION_FAILED,
    INVALID_HEADER,
    OK,
    TIME_OUT,
};

struct ChannelError {
    ChannelError() = default;

    // NOTE: we are not marking these as explicit
    // to reduce code clutter. Since having
    // to write ChannelError{} for every time you
    // want to  create an is a pain
    ChannelError(const char* what)
        : m_code(ChannelErrorCode::ERRNO), m_what(what)
    {
    }

    ChannelError(std::string what)
        : m_code(ChannelErrorCode::ERRNO), m_what(what)
    {
    }

    ChannelError(ChannelErrorCode code)
        : m_code(code)
    {
    }

    operator ChannelErrorCode() const
    {
        return m_code;
    }

    [[nodiscard]] std::string what() const
    {
        switch (m_code) {
        case ChannelErrorCode::END_OF_FILE:
            return "end of file reached";
        case ChannelErrorCode::HUNG_UP:
            return "connection hung up";
        case ChannelErrorCode::DESERIALIZATION_FAILED:
            return "deserialization failed";
        case ChannelErrorCode::INVALID_HEADER:
            return "invalid header encountered";
        case ChannelErrorCode::TIME_OUT:
            return "connection timed out";
        case ChannelErrorCode::OK:
            return "ok";
        case ChannelErrorCode::ERRNO:
            if (m_what) {
                return *m_what;
            }

            ABORT("unreachable");
        }

        return "";
    }

   private:
    ChannelErrorCode m_code { ChannelErrorCode::OK };

    std::optional<std::string> m_what {};
};

// The basic premise of a channel is that it aggregates
// all the necessary machinery to send and receive
// over a socket. The main benefit of this
// is that the caller does not need to worry about
// having to figure how much it should read from the socket.
// Since all of this is done in the channel class.
// hence the Channel just returns a byte string.

class Channel {
   public:
    Channel() = default;

    explicit Channel(IPv4Socket& conn_sock)
        : m_conn_sock { conn_sock }
    {
    }

    explicit Channel(IPv4SocketRef& conn_sock)
        : m_conn_sock { conn_sock }
    {
    }

    Channel(const Channel& other) = default;

    Channel& operator=(const Channel& other) = default;

    [[nodiscard]] std::pair<ByteString, ChannelError> read(int time_out = -1)
    {
        PollResult poll_result {};

        try {
            poll_result = m_conn_sock.poll({ POLLIN }, time_out);
        } catch (std::runtime_error& error) {
            return std::make_pair(ByteString {}, error.what());
        }

        if (poll_result.has_timed_out()) {
            return std::make_pair(ByteString {}, ChannelErrorCode::TIME_OUT);
        }

        if (poll_result.is_hup()) {
            return std::make_pair(ByteString {}, ChannelErrorCode::HUNG_UP);
        }

        ReadResult read_result {};

        try {
            read_result = m_conn_sock.read(Header::size());
        } catch (std::runtime_error& error) {
            return std::make_pair(ByteString {}, error.what());
        }

        if (read_result.is_eof()) {
            return std::make_pair(ByteString {}, ChannelErrorCode::END_OF_FILE);
        }

        auto [header, status] = deserialize<Header>(read_result.get_bytes());

        if (status != DeserializeErrorCode::OK) {
            return std::make_pair(
                ByteString {},
                ChannelErrorCode::DESERIALIZATION_FAILED
            );
        }

        // This is a minor sanity check before proceeding to
        // ensure that the header is well-formed although
        // ideally we'd use a checksum, but we already
        // have a lot of guarantees since we are using
        // TCP streams.

        if (header.magic_bytes != MAGIC_BYTES) {
            return std::make_pair(
                ByteString {},
                ChannelErrorCode::INVALID_HEADER
            );
        }

        try {
            read_result = m_conn_sock.read(header.payload_size);
        } catch (std::runtime_error& error) {
            return std::make_pair(ByteString {}, error.what());
        }

        if (read_result.is_eof()) {
            return std::make_pair(ByteString {}, ChannelErrorCode::END_OF_FILE);
        }

        return std::make_pair(read_result.get_bytes(), ChannelErrorCode::OK);
    }

    [[nodiscard]] ChannelError write(const ByteString& payload)
    {
        ByteString header { serialize(Header { MAGIC_BYTES, payload.size() }) };

        ByteString packet {};

        packet.reserve(header.size() + payload.size());

        for (auto& byte : header) {
            packet.push_back(byte);
        }

        for (auto& byte : payload) {
            packet.push_back(byte);
        }

        PollResult poll_result {};

        try {
            poll_result = m_conn_sock.poll({ POLLOUT });
        } catch (std::runtime_error& error) {
            return error.what();
        }

        if (poll_result.has_timed_out()) {
            return ChannelErrorCode::TIME_OUT;
        }

        if (poll_result.is_hup()) {
            return ChannelErrorCode::HUNG_UP;
        }

        try {
            m_conn_sock.write(packet);
        } catch (std::runtime_error& error) {
            return error.what();
        }

        return ChannelErrorCode::OK;
    }

   private:
    IPv4SocketRef m_conn_sock {};
};
