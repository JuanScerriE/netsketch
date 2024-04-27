#pragma once

// common
#include "bytes.hpp"
#include "overload.hpp"
#include "types.hpp"

// cstd
#include <cstring>

// std
#include <exception>
#include <string>
#include <variant>

// fmt
#include <fmt/core.h>

class ExceededSizeError : public std::exception {
   public:
    explicit ExceededSizeError(
        std::size_t expected_size,
        std::size_t exceeded_by
    )
        : m_expected_size { expected_size }, m_exceeded_by { exceeded_by }
    {
    }

    [[nodiscard]] std::size_t expected_size() const
    {
        return m_expected_size;
    }

    [[nodiscard]] std::size_t exceeded_by() const
    {
        return m_exceeded_by;
    }

   private:
    std::size_t m_expected_size;
    std::size_t m_exceeded_by;
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
            throw ExceededSizeError(
                m_bytes.size(),
                m_offset + sizeof(T) - m_bytes.size()
            );
        }

        ByteArray<sizeof(T)> bytes {};

        for (size_t i = 0; i < sizeof(T); i++) {
            bytes[i] = m_bytes[m_offset + i];
        }

        m_offset += sizeof(T);

        return from_bytes<T>(bytes);
    }

    [[nodiscard]] size_t current_offset() const
    {
        return m_offset;
    }

    [[nodiscard]] size_t expected_size() const
    {
        return m_bytes.size();
    }

   private:
    size_t m_offset { 0 };

    ByteVector m_bytes {};
};

using Serializable = std::variant<Payload>;

class Serialize {
   public:
    explicit Serialize(Serializable arg)
    {
        std::visit(
            overload {
                [this](Payload& arg) {
                    ser(arg);
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
            overload {
                [this](Adopt& arg) {
                    m_fserial.write(PayloadType::ADOPT);
                    ser(arg);
                },
                [this](Decline& arg) {
                    m_fserial.write(PayloadType::DECLINE);
                    ser(arg);
                },
                [this](Accept& arg) {
                    m_fserial.write(PayloadType::ACCEPT);
                    ser(arg);
                },
                [this](Username& arg) {
                    m_fserial.write(PayloadType::USERNAME);
                    ser(arg);
                },
                [this](TaggedDrawVector& arg) {
                    m_fserial.write(PayloadType::TAGGED_DRAW_VECTOR);
                    ser(arg);
                },
                [this](TaggedAction& arg) {
                    m_fserial.write(PayloadType::TAGGED_ACTION);
                    ser(arg);
                },
            },
            arg
        );
    }

    void ser(Adopt& arg)
    {
        ser(arg.username);
    }

    void ser(Decline& arg)
    {
        ser(arg.reason);
    }

    void ser(Accept&)
    {
    }

    void ser(Username& arg)
    {
        ser(arg.username);
    }

    void ser(TaggedDrawVector& arg)
    {
        m_fserial.write(arg.size());

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
            overload {
                [this](Clear& arg) {
                    m_fserial.write(ActionType::CLEAR);
                    ser(arg);
                },
                [this](Undo& arg) {
                    m_fserial.write(ActionType::UNDO);
                    ser(arg);
                },
                [this](Delete& arg) {
                    m_fserial.write(ActionType::DELETE);
                    ser(arg);
                },
                [this](Select& arg) {
                    m_fserial.write(ActionType::SELECT);
                    ser(arg);
                },
                [this](Draw& arg) {
                    m_fserial.write(ActionType::DRAW);
                    ser(arg);
                },
            },
            arg
        );
    }

    void ser(Clear& arg)
    {
        m_fserial.write(arg.qualifier);
    }

    void ser(Undo&)
    {
    }

    void ser(Delete& arg)
    {
        m_fserial.write(arg.id);
    }

    void ser(Select& arg)
    {
        m_fserial.write(arg.id);

        ser(arg.draw);
    }

    void ser(Draw& arg)
    {
        std::visit(
            overload {
                [this](TextDraw& arg) {
                    m_fserial.write(DrawType::TEXT);
                    ser(arg);
                },
                [this](CircleDraw& arg) {
                    m_fserial.write(DrawType::CIRCLE);
                    ser(arg);
                },
                [this](RectangleDraw& arg) {
                    m_fserial.write(DrawType::RECTANGLE);
                    ser(arg);
                },
                [this](LineDraw& arg) {
                    m_fserial.write(DrawType::LINE);
                    ser(arg);
                },
            },
            arg
        );
    }

    void ser(TextDraw& arg)
    {
        m_fserial.write(arg.colour);
        m_fserial.write(arg.x);
        m_fserial.write(arg.y);

        ser(arg.string);
    }

    void ser(CircleDraw& arg)
    {
        m_fserial.write(arg);
    }

    void ser(RectangleDraw& arg)
    {
        m_fserial.write(arg);
    }

    void ser(LineDraw& arg)
    {
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

class UnexpectedTypeError : public std::exception {
   public:
    explicit UnexpectedTypeError(uint32_t type)
        : m_type { type }
    {
    }

    [[nodiscard]] uint32_t type() const
    {
        return m_type;
    }

   private:
    uint32_t m_type {};
};

enum class DeserializeErrorCode {
    EXCEEDED_SIZE,
    OK,
    UNEXPECTED_TYPE,
};

struct DeserializeError {
    DeserializeError(ExceededSizeError error)
        : m_code(DeserializeErrorCode::EXCEEDED_SIZE), m_what(error)
    {
    }

    DeserializeError(UnexpectedTypeError error)
        : m_code(DeserializeErrorCode::UNEXPECTED_TYPE), m_what(error)
    {
    }

    DeserializeError(DeserializeErrorCode code)
        : m_code(code)
    {
    }

    operator DeserializeErrorCode() const
    {
        return m_code;
    }

    [[nodiscard]] std::string what() const
    {
        return std::visit(
            overload {
                [](const UnexpectedTypeError& error) {
                    return fmt::format("unexpected type {}", error.type());
                },
                [](const ExceededSizeError& error) {
                    return fmt::format(
                        "exceeded {} by {} (bytes)",
                        error.expected_size(),
                        error.exceeded_by()
                    );
                },
                [](const std::monostate&) {
                    return std::string {};
                },
            },
            m_what
        );
    }

   private:
    DeserializeErrorCode m_code { DeserializeErrorCode::OK };

    std::variant<std::monostate, UnexpectedTypeError, ExceededSizeError>
        m_what { std::monostate {} };
};

class Deserialize {
   public:
    explicit Deserialize(ByteVector bytes)
        : m_bserial(std::move(bytes))
    {
    }

    [[nodiscard]] std::pair<Payload, DeserializeError> payload() noexcept
    {
        try {
            return std::make_pair(deser_payload(), DeserializeErrorCode::OK);
        } catch (UnexpectedTypeError& error) {
            return std::make_pair(Payload {}, error);
        } catch (ExceededSizeError& error) {
            return std::make_pair(Payload {}, error);
        }
    }

   private:
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
            throw UnexpectedTypeError(static_cast<uint32_t>(type));
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

        if (vector_size + m_bserial.current_offset()
            > m_bserial.expected_size()) {
            throw ExceededSizeError(
                m_bserial.expected_size(),
                vector_size + m_bserial.current_offset()
                    - m_bserial.expected_size()
            );
        }

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
            throw UnexpectedTypeError(static_cast<uint32_t>(type));
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
            throw UnexpectedTypeError(static_cast<uint32_t>(type));
        }
    }

    std::string deser_string()
    {
        auto string_size = m_bserial.read<size_t>();

        if (string_size + m_bserial.current_offset()
            > m_bserial.expected_size()) {
            throw ExceededSizeError(
                m_bserial.expected_size(),
                string_size + m_bserial.current_offset()
                    - m_bserial.expected_size()
            );
        }

        std::string string {};

        string.reserve(string_size);

        for (size_t i = 0; i < string_size; i++) {
            string.push_back(m_bserial.read<char>());
        }

        return string;
    }

    BSerial m_bserial;
};
