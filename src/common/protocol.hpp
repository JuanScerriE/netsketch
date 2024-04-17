#pragma once

// std
#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

// util
#include <serial.hpp>
#include <utils.hpp>

// common
#include <abort.hpp>

#define MAGIC_BYTES (static_cast<uint16_t>(0x2003))

namespace prot {

struct colour_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct line_draw_t {
    colour_t colour;
    int x0;
    int y0;
    int x1;
    int y1;
};

struct rectangle_draw_t {
    colour_t colour;
    int x;
    int y;
    int w;
    int h;
};

struct circle_draw_t {
    colour_t colour;
    int x;
    int y;
    float r;
};

struct text_draw_t {
    colour_t colour;
    int x;
    int y;
    std::string string;
};

enum class tool_e : uint16_t {
    LINE,
    RECTANGLE,
    CIRCLE,
    TEXT
};

using draw_t = std::variant<line_draw_t, rectangle_draw_t,
    circle_draw_t, text_draw_t>;

struct delete_t {
    uint32_t id;
};

struct undo_t { };

enum class qualifier_e : uint8_t { ALL, MINE };

struct clear_t {
    qualifier_e quailifier;
};

enum class command_type_e : uint16_t {
    DRAW,
    DELETE,
    UNDO,
    CLEAR
};

struct tagged_command_t {
    std::string username;
    std::variant<draw_t, delete_t, undo_t, clear_t> command;
};

enum class payload_type_e : uint16_t { COMMAND };

using payload_t = std::variant<tagged_command_t>;

struct header_t {
    uint16_t magic_bytes;
    size_t payload_size;

    [[nodiscard]] bool is_malformed() const
    {
        return magic_bytes != MAGIC_BYTES;
    }
};

class serialize_t {
public:
    explicit serialize_t(payload_t payload)
    {
        ser_payload(payload);
    }

    void ser_payload(payload_t& payload)
    {
        if (std::holds_alternative<tagged_command_t>(
                payload)) {
            m_fserial.write(payload_type_e::COMMAND);

            auto& command
                = std::get<tagged_command_t>(payload);

            ser_command(command);

            return;
        }

        Abort("unreachable");
    }

    void ser_command(tagged_command_t& tagged_command)
    {
        m_fserial.write(tagged_command.username.size());

        for (char character : tagged_command.username) {
            m_fserial.write(character);
        }

        if (std::holds_alternative<draw_t>(
                tagged_command.command)) {
            m_fserial.write(command_type_e::DRAW);

            auto& draw
                = std::get<draw_t>(tagged_command.command);

            ser_draw(draw);

            return;
        }

        if (std::holds_alternative<delete_t>(
                tagged_command.command)) {
            m_fserial.write(command_type_e::DELETE);

            auto& delete_ = std::get<delete_t>(
                tagged_command.command);

            m_fserial.write(delete_.id);

            return;
        }

        if (std::holds_alternative<undo_t>(
                tagged_command.command)) {
            m_fserial.write(command_type_e::UNDO);

            return;
        }

        if (std::holds_alternative<clear_t>(
                tagged_command.command)) {
            m_fserial.write(command_type_e::CLEAR);

            auto& clear
                = std::get<clear_t>(tagged_command.command);

            m_fserial.write(clear.quailifier);

            return;
        }

        Abort("unreachable");
    }

    void ser_draw(draw_t& draw)
    {
        if (std::holds_alternative<line_draw_t>(draw)) {
            m_fserial.write(tool_e::LINE);

            auto& line = std::get<line_draw_t>(draw);

            m_fserial.write(line);

            return;
        }

        if (std::holds_alternative<rectangle_draw_t>(
                draw)) {
            m_fserial.write(tool_e::RECTANGLE);

            auto& rectangle
                = std::get<rectangle_draw_t>(draw);

            m_fserial.write(rectangle);

            return;
        }

        if (std::holds_alternative<circle_draw_t>(draw)) {
            m_fserial.write(tool_e::CIRCLE);

            auto& circle = std::get<circle_draw_t>(draw);

            m_fserial.write(circle);

            return;
        }

        if (std::holds_alternative<text_draw_t>(draw)) {
            m_fserial.write(tool_e::TEXT);

            auto& text = std::get<text_draw_t>(draw);

            m_fserial.write(text.colour);
            m_fserial.write(text.x);
            m_fserial.write(text.y);
            m_fserial.write(text.string.length());

            for (auto character : text.string) {
                m_fserial.write(character);
            }

            return;
        }

        Abort("unreachable");
    }

    [[nodiscard]] util::byte_vector bytes() const
    {
        return m_fserial.bytes();
    }

private:
    util::fserial_t m_fserial {};
};

class deserialize_t {
public:
    explicit deserialize_t(util::byte_vector bytes)
        : m_bserial(bytes)
    {
        deser_payload();
    }

    void deser_payload()
    {
        if (std::holds_alternative<tagged_command_t>(
                payload)) {
            serial.write(payload_type_e::COMMAND);

            auto& command
                = std::get<tagged_command_t>(payload);

            ser_command(command);

            return;
        }

        Abort("unreachable");
    }

    void ser_command(tagged_command_t& tagged_command)
    {
        serial.write(tagged_command.username.size());

        for (char character : tagged_command.username) {
            serial.write(character);
        }

        if (std::holds_alternative<draw_t>(
                tagged_command.command)) {
            serial.write(command_type_e::DRAW);

            auto& draw
                = std::get<draw_t>(tagged_command.command);

            ser_draw(draw);

            return;
        }

        if (std::holds_alternative<delete_t>(
                tagged_command.command)) {
            serial.write(command_type_e::DELETE);

            auto& delete_ = std::get<delete_t>(
                tagged_command.command);

            serial.write(delete_.id);

            return;
        }

        if (std::holds_alternative<undo_t>(
                tagged_command.command)) {
            serial.write(command_type_e::UNDO);

            return;
        }

        if (std::holds_alternative<clear_t>(
                tagged_command.command)) {
            serial.write(command_type_e::CLEAR);

            auto& clear
                = std::get<clear_t>(tagged_command.command);

            serial.write(clear.quailifier);

            return;
        }

        Abort("unreachable");
    }

    void ser_draw(draw_t& draw)
    {
        if (std::holds_alternative<line_draw_t>(draw)) {
            serial.write(tool_e::LINE);

            auto& line = std::get<line_draw_t>(draw);

            serial.write(line);

            return;
        }

        if (std::holds_alternative<rectangle_draw_t>(
                draw)) {
            serial.write(tool_e::RECTANGLE);

            auto& rectangle
                = std::get<rectangle_draw_t>(draw);

            serial.write(rectangle);

            return;
        }

        if (std::holds_alternative<circle_draw_t>(draw)) {
            serial.write(tool_e::CIRCLE);

            auto& circle = std::get<circle_draw_t>(draw);

            serial.write(circle);

            return;
        }

        if (std::holds_alternative<text_draw_t>(draw)) {
            serial.write(tool_e::TEXT);

            auto& text = std::get<text_draw_t>(draw);

            serial.write(text.colour);
            serial.write(text.x);
            serial.write(text.y);
            serial.write(text.string.length());

            for (auto character : text.string) {
                serial.write(character);
            }

            return;
        }

        Abort("unreachable");
    }

    [[nodiscard]] util::byte_vector bytes() const
    {
        return serial.bytes();
    }

private:
    util::bserial_t m_bserial;
};

} // namespace prot
