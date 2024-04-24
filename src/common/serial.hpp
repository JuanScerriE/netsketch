#pragma once

// cstd
#include <cstring>

// std
#include <array>
#include <exception>
#include <string>
#include <variant>
#include <vector>

// types
#include <types.hpp>

template <std::size_t N>
using ByteArray = std::array<std::byte, N>;

using ByteVector = std::vector<std::byte>;

template <typename T>
ByteArray<sizeof(T)> to_bytes(T value)
{
    ByteArray<sizeof(T)> bytes {};

    std::memcpy(&bytes, &value, sizeof(T));

    return bytes;
}

template <typename T>
T from_bytes(ByteArray<sizeof(T)> bytes)
{
    T value {};

    std::memcpy(&value, &bytes, sizeof(T));

    return value;
}

class SerialError : public std::exception {
   public:
    explicit SerialError(std::string message)
        : m_message(std::move(message))
    {
    }

    [[nodiscard]] const char* what() const noexcept override
    {
        return m_message.c_str();
    }

   private:
    std::string m_message {};
};

class FSerial {
   public:
    template <typename T>
    void write(T value)
    {
        static_assert(
            std::is_trivially_copyable_v<T>,
            "cannot write non-trivially copyable type"
        );

        for (auto& byte : to_bytes(value)) {
            m_bytes.emplace_back(byte);
        }
    }

    [[nodiscard]] ByteVector bytes() const
    {
        return m_bytes;
    }

   private:
    ByteVector m_bytes {};
};

class BSerial {
   public:
    explicit BSerial(ByteVector bytes)
        : m_bytes(std::move(bytes))
    {
    }

    template <typename T>
    T read()
    {
        static_assert(
            std::is_trivially_copyable_v<T>,
            "cannot read non-trivially copyable type"
        );

        if (m_offset + sizeof(T) > m_bytes.size()) {
            throw SerialError("exceeded vector size");
        }

        ByteArray<sizeof(T)> bytes {};

        for (size_t i = 0; i < sizeof(T); i++) {
            bytes[i] = m_bytes[m_offset + i];
        }

        m_offset += sizeof(T);

        return from_bytes<T>(bytes);
    }

   private:
    size_t m_offset { 0 };

    ByteVector m_bytes {};
};

using Serializable = std::variant<Payload>;

template <typename... Types>
struct overloaded : Types... {
    using Types::operator()...;
};

template <typename... Types>
overloaded(Types...) -> overloaded<Types...>;

class Serialize {
   public:
    explicit Serialize(Serializable arg)
    {
        std::visit(
            overloaded {
                [this](Payload& arg) {
                    this->ser(arg);
                },
            },
            arg
        );
    }

    [[nodiscard]] ByteVector bytes() const
    {
        return m_fserial.bytes();
    }

   private:
    void ser(Payload& arg)
    {
        std::visit(
            overloaded {
                [this](Adopt& arg) {
                    this->ser(arg);
                },
                [this](Decline& arg) {
                    this->ser(arg);
                },
                [this](Accept& arg) {
                    this->ser(arg);
                },
                [this](Username& arg) {
                    this->ser(arg);
                },
                [this](TaggedDrawVector& arg) {
                    this->ser(arg);
                },
                [this](TaggedDraw& arg) {
                    this->ser(arg);
                },
                [this](TaggedAction& arg) {
                    this->ser(arg);
                },
            },
            arg
        );
    }

    void ser(Adopt& arg)
    {
        m_fserial.write(PayloadType::ADOPT);

        ser(arg.username);
    }

    void ser(Decline& arg)
    {
        m_fserial.write(PayloadType::DECLINE);

        ser(arg.reason);
    }

    void ser(Accept&)
    {
        m_fserial.write(PayloadType::DECLINE);
    }

    void ser(Username& arg)
    {
        m_fserial.write(PayloadType::USERNAME);

        ser(arg.username);
    }

    void ser(TaggedDrawVector& arg)
    {
        m_fserial.write(PayloadType::TAGGED_DRAW_VECTOR);

        for (auto& arg_ : arg) {
            ser(arg_);
        }
    }

    void ser(TaggedDraw& arg)
    {
        m_fserial.write(arg.adopted);

        ser(arg.username);
        ser(arg.draw);
    }

    void ser(TaggedAction& arg)
    {
        ser(arg.username);
        ser(arg.action);
    }

    void ser(Action& arg)
    {
        std::visit(
            overloaded {
                [this](Clear& arg) {
                    this->ser(arg);
                },
                [this](Undo& arg) {
                    this->ser(arg);
                },
                [this](Delete& arg) {
                    this->ser(arg);
                },
                [this](Select& arg) {
                    this->ser(arg);
                },
                [this](Draw& arg) {
                    this->ser(arg);
                },
            },
            arg
        );
    }

    void ser(Clear& arg)
    {
        m_fserial.write(ActionType::CLEAR);
        m_fserial.write(arg.qualifier);
    }

    void ser(Undo&)
    {
        m_fserial.write(ActionType::UNDO);
    }

    void ser(Delete& arg)
    {
        m_fserial.write(ActionType::DELETE);
        m_fserial.write(arg.id);
    }

    void ser(Select& arg)
    {
        m_fserial.write(ActionType::SELECT);
        m_fserial.write(arg.id);

        ser(arg.draw);
    }

    void ser(Draw& arg)
    {
        m_fserial.write(ActionType::DRAW);

        std::visit(
            overloaded {
                [this](TextDraw& arg) {
                    this->ser(arg);
                },
                [this](CircleDraw& arg) {
                    this->ser(arg);
                },
                [this](RectangleDraw& arg) {
                    this->ser(arg);
                },
                [this](LineDraw& arg) {
                    this->ser(arg);
                },
            },
            arg
        );
    }

    void ser(TextDraw& arg)
    {
        m_fserial.write(DrawType::TEXT);
        m_fserial.write(arg.colour);
        m_fserial.write(arg.x);
        m_fserial.write(arg.y);

        ser(arg.string);
    }

    void ser(CircleDraw& arg)
    {
        m_fserial.write(DrawType::CIRCLE);
        m_fserial.write(arg);
    }

    void ser(RectangleDraw& arg)
    {
        m_fserial.write(DrawType::RECTANGLE);
        m_fserial.write(arg);
    }

    void ser(LineDraw& arg)
    {
        m_fserial.write(DrawType::LINE);
        m_fserial.write(arg);
    }

    void ser(const std::string& arg)
    {
        m_fserial.write(arg.size());

        for (char character : arg) {
            m_fserial.write(character);
        }
    }

   private:
    FSerial m_fserial {};
};

class Deserialize {
   public:
    explicit Deserialize(ByteVector bytes)
        : m_bserial(std::move(bytes))
    {
    }

    [[nodiscard]] PayloadHeader payload_header()
    {
        return deser_payload_header();
    }

    [[nodiscard]] Payload payload()
    {
        return deser_payload();
    }

   private:
    PayloadHeader deser_payload_header()
    {
        return { m_bserial.read<std::uint16_t>(),
                 m_bserial.read<std::size_t>() };
    }

    Payload deser_payload()
    {
        auto type = m_bserial.read<PayloadType>();

        switch (type) {
        case PayloadType::ADOPT:
            return deser_adopt();
        case PayloadType::DECLINE:
            return deser_decline();
        case PayloadType::ACCEPT:
            return deser_accept();
        case PayloadType::USERNAME:
            return deser_username();
        case PayloadType::TAGGED_DRAW_VECTOR:
            return deser_tagged_draw_vector();
        case PayloadType::TAGGED_ACTION:
            return deser_tagged_action();
        default:
            throw SerialError("unexpected type");
        }
    }

    Adopt deser_adopt()
    {
        return { deser_string() };
    }

    Decline deser_decline()
    {
        return { deser_string() };
    }

    Accept deser_accept()
    {
        return {};
    }

    Username deser_username()
    {
        return { deser_string() };
    }

    TaggedDrawVector deser_tagged_draw_vector()
    {
        auto vector_size = m_bserial.read<size_t>();

        TaggedDrawVector vector {};

        vector.reserve(vector_size);

        for (size_t i = 0; i < vector_size; i++) {
            vector.push_back(deser_tagged_draw());
        }

        return vector;
    }

    TaggedDraw deser_tagged_draw()
    {
        return { m_bserial.read<bool>(), deser_string(), deser_draw() };
    }

    Draw deser_draw()
    {
        auto type = m_bserial.read<DrawType>();

        switch (type) {
        case DrawType::LINE:
            return m_bserial.read<LineDraw>();
        case DrawType::RECTANGLE:
            return m_bserial.read<RectangleDraw>();
        case DrawType::CIRCLE:
            return m_bserial.read<CircleDraw>();
        case DrawType::TEXT: {
            return TextDraw { m_bserial.read<Colour>(),
                              m_bserial.read<decltype(TextDraw::x)>(),
                              m_bserial.read<decltype(TextDraw::y)>(),
                              deser_string() };
        }
        default:
            throw SerialError("unexpected type");
        }
    }

    TaggedAction deser_tagged_action()
    {
        return { deser_string(), deser_action() };
    }

    Action deser_action()
    {
        auto type = m_bserial.read<ActionType>();

        switch (type) {
        case ActionType::DRAW:
            return deser_draw();
        case ActionType::SELECT:
            return Select { m_bserial.read<decltype(Select::id)>(),
                            deser_draw() };
        case ActionType::DELETE:
            return Delete { m_bserial.read<decltype(Delete::id)>() };
        case ActionType::UNDO:
            return Undo {};
        case ActionType::CLEAR:
            return Clear { m_bserial.read<Qualifier>() };
        default:
            throw SerialError("unexpected type");
        }
    }

    std::string deser_string()
    {
        auto string_size = m_bserial.read<size_t>();

        std::string string {};

        string.reserve(string_size);

        for (size_t i = 0; i < string_size; i++) {
            string.push_back(m_bserial.read<char>());
        }

        return string;
    }

    BSerial m_bserial;
};
