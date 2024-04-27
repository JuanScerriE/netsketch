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

#include <types.hpp>

using Serializable = std::variant<
    PayloadHeader,
    Payload,
    TaggedCommand,
    TaggedDraw,
    TaggedDrawList,
    Adopt>;

class serialize_t {
   public:
    explicit serialize_t(Payload payload)
    {
        ser_payload(payload);
    }

    void ser_payload(Payload& payload)
    {
        if (std::holds_alternative<TaggedCommand>(payload)) {
            m_fserial.write(payload_type_e::TAGGED_COMMAND);

            auto& command = std::get<TaggedCommand>(payload);

            ser_tagged_command(command);

            return;
        }

        if (std::holds_alternative<TaggedDraw>(payload)) {
            m_fserial.write(payload_type_e::TAGGED_DRAW);

            auto& tagged_draw = std::get<TaggedDraw>(payload);

            ser_tagged_draw(tagged_draw);

            return;
        }

        if (std::holds_alternative<TaggedDrawList>(payload)) {
            m_fserial.write(payload_type_e::TAGGED_DRAW_LIST);

            auto& tagged_draw_list = std::get<TaggedDrawList>(payload);

            m_fserial.write(tagged_draw_list.size());

            for (auto& tagged_draw : tagged_draw_list) {
                ser_tagged_draw(tagged_draw);
            }

            return;
        }

        if (std::holds_alternative<Adopt>(payload)) {
            m_fserial.write(payload_type_e::ADOPT);

            auto& adopt = std::get<Adopt>(payload);

            m_fserial.write(adopt.username.size());

            for (char character : adopt.username) {
                m_fserial.write(character);
            }

            return;
        }

        ABORT("unreachable");
    }

    void ser_tagged_draw(TaggedDraw& tagged_draw)
    {
        m_fserial.write(tagged_draw.adopted);

        m_fserial.write(tagged_draw.username.size());

        for (char character : tagged_draw.username) {
            m_fserial.write(character);
        }

        ser_draw(tagged_draw.draw);
    }

    void ser_tagged_command(TaggedCommand& tagged_command)
    {
        m_fserial.write(tagged_command.username.size());

        for (char character : tagged_command.username) {
            m_fserial.write(character);
        }

        if (std::holds_alternative<draw_t>(tagged_command.command)) {
            m_fserial.write(command_type_e::DRAW);

            auto& draw = std::get<draw_t>(tagged_command.command);

            ser_draw(draw);

            return;
        }

        if (std::holds_alternative<select_t>(tagged_command.command)) {
            m_fserial.write(command_type_e::SELECT);

            auto& select = std::get<select_t>(tagged_command.command);

            m_fserial.write(select.id);

            ser_draw(select.draw);

            return;
        }

        if (std::holds_alternative<delete_t>(tagged_command.command)) {
            m_fserial.write(command_type_e::DELETE);

            auto& delete_ = std::get<delete_t>(tagged_command.command);

            m_fserial.write(delete_.id);

            return;
        }

        if (std::holds_alternative<undo_t>(tagged_command.command)) {
            m_fserial.write(command_type_e::UNDO);

            return;
        }

        if (std::holds_alternative<clear_t>(tagged_command.command)) {
            m_fserial.write(command_type_e::CLEAR);

            auto& clear = std::get<clear_t>(tagged_command.command);

            m_fserial.write(clear.quailifier);

            return;
        }

        ABORT("unreachable");
    }

    void ser_draw(draw_t& draw)
    {
        if (std::holds_alternative<line_draw_t>(draw)) {
            m_fserial.write(tool_e::LINE);

            auto& line = std::get<line_draw_t>(draw);

            m_fserial.write(line);

            return;
        }

        if (std::holds_alternative<rectangle_draw_t>(draw)) {
            m_fserial.write(tool_e::RECTANGLE);

            auto& rectangle = std::get<rectangle_draw_t>(draw);

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

        ABORT("unreachable");
    }

    [[nodiscard]] util::ByteVector bytes() const
    {
        return m_fserial.bytes();
    }

   private:
    util::FSerial m_fserial {};
};

class deserialize_t {
   public:
    explicit deserialize_t(util::ByteVector bytes)
        : m_bserial(std::move(bytes))
    {
        m_payload = deser_payload();
    }

    [[nodiscard]] Payload payload() const
    {
        return m_payload;
    }

    Payload deser_payload()
    {
        auto payload_type = m_bserial.read<payload_type_e>();

        switch (payload_type) {
        case payload_type_e::TAGGED_COMMAND:
            return deser_tagged_command();
        case payload_type_e::TAGGED_DRAW:
            return deser_tagged_draw();
        case payload_type_e::TAGGED_DRAW_LIST:
            return deser_tagged_draw_list();
        case payload_type_e::ADOPT:
            return deser_adopt();
        default:
            throw util::serial_error_t("unexpected type");
        }
    }

    Adopt deser_adopt()
    {
        auto username_size = m_bserial.read<size_t>();

        std::string username {};

        for (size_t i = 0; i < username_size; i++) {
            username.push_back(m_bserial.read<char>());
        }

        return { username };
    }

    TaggedDrawList deser_tagged_draw_list()
    {
        auto list_size = m_bserial.read<size_t>();

        TaggedDrawList list {};

        for (size_t i = 0; i < list_size; i++) {
            list.push_back(deser_tagged_draw());
        }

        return list;
    }

    TaggedDraw deser_tagged_draw()
    {
        auto adopted = m_bserial.read<bool>();

        auto username_size = m_bserial.read<size_t>();

        std::string username {};

        for (size_t i = 0; i < username_size; i++) {
            username.push_back(m_bserial.read<char>());
        }

        return { adopted, username, deser_draw() };
    }

    TaggedCommand deser_tagged_command()
    {
        auto username_size = m_bserial.read<size_t>();

        std::string username {};

        for (size_t i = 0; i < username_size; i++) {
            username.push_back(m_bserial.read<char>());
        }

        auto command_type = m_bserial.read<command_type_e>();

        command_t command {};

        switch (command_type) {
        case command_type_e::DRAW:
            command = deser_draw();
            break;
        case command_type_e::SELECT:
            command = select_t { m_bserial.read<decltype(select_t::id)>(),
                                 deser_draw() };
            break;

        case command_type_e::DELETE:
            command = delete_t { m_bserial.read<decltype(delete_t::id)>() };
            break;
        case command_type_e::UNDO:
            command = undo_t {};
            break;
        case command_type_e::CLEAR:
            command = clear_t { m_bserial.read<qualifier_e>() };
            break;
        default:
            throw util::serial_error_t("unexpected type");
        }

        return { username, command };
    }

    draw_t deser_draw()
    {
        auto tool = m_bserial.read<tool_e>();

        switch (tool) {
        case tool_e::LINE:
            return m_bserial.read<line_draw_t>();
        case tool_e::RECTANGLE:
            return m_bserial.read<rectangle_draw_t>();
        case tool_e::CIRCLE:
            return m_bserial.read<circle_draw_t>();
        case tool_e::TEXT: {
            auto colour = m_bserial.read<colour_t>();
            auto x = m_bserial.read<decltype(text_draw_t::x)>();
            auto y = m_bserial.read<decltype(text_draw_t::y)>();
            auto size = m_bserial.read<size_t>();

            std::string text {};

            for (size_t i = 0; i < size; i++) {
                text.push_back(m_bserial.read<char>());
            }

            return text_draw_t { colour, x, y, text };
        }
        default:
            throw util::serial_error_t("unexpected type");
        }
    }

   private:
    Payload m_payload {};

    util::BSerial m_bserial;
};

} // namespace prot