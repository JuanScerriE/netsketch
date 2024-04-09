#pragma once

// parl
#include <parl/Core.hpp>
#include <parl/Value.hpp>

// std
#include <optional>

namespace PArL {

class Token {
   public:
    enum class Type {
        // one or two character tokens
        LEFT_BRACK,
        RIGHT_BRACK,
        STAR,
        SLASH,
        PLUS,
        MINUS,
        LESS,
        GREATER,
        EQUAL_EQUAL,
        BANG_EQUAL,
        LESS_EQUAL,
        GREATER_EQUAL,
        COMMA,
        LEFT_PAREN,
        RIGHT_PAREN,
        EQUAL,
        COLON,
        SEMICOLON,
        ARROW,
        LEFT_BRACE,
        RIGHT_BRACE,

        // types
        FLOAT_TYPE,
        INTEGER_TYPE,
        BOOL_TYPE,
        COLOR_TYPE,

        // literals
        FLOAT,
        INTEGER,
        COLOR,
        BOOL,
        BUILTIN,
        IDENTIFIER,

        // keywords
        AND,
        OR,
        NOT,
        AS,
        LET,
        RETURN,
        IF,
        ELSE,
        FOR,
        WHILE,
        FUN,

        // special tokens
        COMMENT,
        WHITESPACE,
        END_OF_FILE,
    };

    Token();

    Token(
        int line,
        int column,
        std::string lexeme,
        Type type
    );

    Token(const Token& other);

    [[nodiscard]] core::Position getPosition() const;
    [[nodiscard]] std::string getLexeme() const;
    [[nodiscard]] Type getType() const;

    template <typename T>
    std::optional<T> asOpt() const {
        if (mValue.has_value()) {
            return std::optional<T>{mValue->as<T>()};
        }

        return {};
    }

    [[nodiscard]] std::string toString() const;

   private:
    [[nodiscard]] bool isContainerType() const;

    void specialise();

    core::Position mPosition;
    std::string mLexeme;
    Type mType;
    std::optional<Value> mValue;
};

}  // namespace PArL
