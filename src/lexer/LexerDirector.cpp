// parl
#include <lexer/LexerDirector.hpp>

namespace PArL {

enum Category {
    LETTER,
    DIGIT,
    HEX,
    SPACE,
    LINEFEED,
    DOT,
    HASH,
    UNDERSCORE,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    LEFT_BRACE,
    RIGHT_BRACE,
    STAR,
    SLASH,
    PLUS,
    MINUS,
    LESS,
    GREATER,
    EQUAL,
    BANG,
    COMMA,
    COLON,
    SEMICOLON,
};

Lexer LexerDirector::buildLexer() {
    LexerBuilder builder{};

    builder
        .addCategory(
            LETTER,
            [](char c) -> bool {
                return ('A' <= c && c <= 'Z') ||
                       ('a' <= c && c <= 'z');
            }
        )
        .addCategory(
            DIGIT,
            [](char c) -> bool {
                return '0' <= c && c <= '9';
            }
        )
        .addCategory(
            HEX,
            [](char c) -> bool {
                return ('0' <= c && c <= '9') ||
                       ('A' <= c && c <= 'F') ||
                       ('a' <= c && c <= 'f');
            }
        )
        .addCategory(
            SPACE,
            [](char c) -> bool {
                return isspace(c) && c != '\n';
            }
        )
        .addCategory(
            LINEFEED,
            [](char c) -> bool {
                return c == '\n';
            }
        )
        .addCategory(
            DOT,
            [](char c) -> bool {
                return c == '.';
            }
        )
        .addCategory(
            HASH,
            [](char c) -> bool {
                return c == '#';
            }
        )
        .addCategory(
            UNDERSCORE,
            [](char c) -> bool {
                return c == '_';
            }
        )
        .addCategory(
            LEFT_PAREN,
            [](char c) -> bool {
                return c == '(';
            }
        )
        .addCategory(
            RIGHT_PAREN,
            [](char c) -> bool {
                return c == ')';
            }
        )
        .addCategory(
            LEFT_BRACKET,
            [](char c) -> bool {
                return c == '[';
            }
        )
        .addCategory(
            RIGHT_BRACKET,
            [](char c) -> bool {
                return c == ']';
            }
        )
        .addCategory(
            LEFT_BRACE,
            [](char c) -> bool {
                return c == '{';
            }
        )
        .addCategory(
            RIGHT_BRACE,
            [](char c) -> bool {
                return c == '}';
            }
        )
        .addCategory(
            COMMA,
            [](char c) -> bool {
                return c == ',';
            }
        )
        .addCategory(
            RIGHT_PAREN,
            [](char c) -> bool {
                return c == ')';
            }
        )
        .addCategory(
            SEMICOLON,
            [](char c) -> bool {
                return c == ';';
            }
        )
        .addCategory(
            STAR,
            [](char c) -> bool {
                return c == '*';
            }
        )
        .addCategory(
            SLASH,
            [](char c) -> bool {
                return c == '/';
            }
        )
        .addCategory(
            PLUS,
            [](char c) -> bool {
                return c == '+';
            }
        )
        .addCategory(
            MINUS,
            [](char c) -> bool {
                return c == '-';
            }
        )
        .addCategory(
            LESS,
            [](char c) -> bool {
                return c == '<';
            }
        )
        .addCategory(
            GREATER,
            [](char c) -> bool {
                return c == '>';
            }
        )
        .addCategory(
            EQUAL,
            [](char c) -> bool {
                return c == '=';
            }
        )
        .addCategory(
            BANG,
            [](char c) -> bool {
                return c == '!';
            }
        )
        .addCategory(
            COMMA,
            [](char c) -> bool {
                return c == ',';
            }
        )
        .addCategory(
            COLON,
            [](char c) -> bool {
                return c == ':';
            }
        )
        .addCategory(SEMICOLON, [](char c) -> bool {
            return c == ';';
        });

    // whitespace
    builder.addTransition(0, {SPACE, LINEFEED}, 1)
        .addTransition(1, {SPACE, LINEFEED}, 1)
        .setStateAsFinal(1, Token::Type::WHITESPACE);

    // identifier
    builder.addTransition(0, {LETTER, UNDERSCORE}, 2)
        .addTransition(2, {LETTER, DIGIT, UNDERSCORE}, 2)
        .setStateAsFinal(2, Token::Type::IDENTIFIER);

    // integers & floats
    builder.addTransition(0, DIGIT, 3)
        .addTransition(3, DIGIT, 3)
        .setStateAsFinal(3, Token::Type::INTEGER)
        .addTransition(3, DOT, 4)
        .addTransition(4, DIGIT, 5)
        .addTransition(5, DIGIT, 5)
        .setStateAsFinal(5, Token::Type::FLOAT);

    // color
    builder.addTransition(0, HASH, 6)
        .addTransition(6, HEX, 7)
        .addTransition(7, HEX, 8)
        .addTransition(8, HEX, 9)
        .addTransition(9, HEX, 10)
        .addTransition(10, HEX, 11)
        .addTransition(11, HEX, 12)
        .setStateAsFinal(12, Token::Type::COLOR);

    // punctuation "(", ")", "{", "}", ";", ",", ":", "[",
    // "]",
    // "*", "+"
    builder.addTransition(0, LEFT_PAREN, 13)
        .addTransition(0, RIGHT_PAREN, 14)
        .addTransition(0, LEFT_BRACE, 15)
        .addTransition(0, RIGHT_BRACE, 16)
        .addTransition(0, SEMICOLON, 17)
        .addTransition(0, COMMA, 18)
        .addTransition(0, COLON, 19)
        .addTransition(0, LEFT_BRACKET, 20)
        .addTransition(0, RIGHT_BRACKET, 21)
        .addTransition(0, STAR, 22)
        .addTransition(0, PLUS, 23)
        .setStateAsFinal(13, Token::Type::LEFT_PAREN)
        .setStateAsFinal(14, Token::Type::RIGHT_PAREN)
        .setStateAsFinal(15, Token::Type::LEFT_BRACE)
        .setStateAsFinal(16, Token::Type::RIGHT_BRACE)
        .setStateAsFinal(17, Token::Type::SEMICOLON)
        .setStateAsFinal(18, Token::Type::COMMA)
        .setStateAsFinal(19, Token::Type::COLON)
        .setStateAsFinal(20, Token::Type::LEFT_BRACK)
        .setStateAsFinal(21, Token::Type::RIGHT_BRACK)
        .setStateAsFinal(22, Token::Type::STAR)
        .setStateAsFinal(23, Token::Type::PLUS);

    // "=", "=="
    builder.addTransition(0, EQUAL, 24)
        .setStateAsFinal(24, Token::Type::EQUAL)
        .addTransition(24, EQUAL, 25)
        .setStateAsFinal(25, Token::Type::EQUAL_EQUAL);

    // "<", "<="
    builder.addTransition(0, LESS, 26)
        .setStateAsFinal(26, Token::Type::LESS)
        .addTransition(26, EQUAL, 27)
        .setStateAsFinal(27, Token::Type::LESS_EQUAL);

    // ">", ">="
    builder.addTransition(0, GREATER, 28)
        .setStateAsFinal(28, Token::Type::GREATER)
        .addTransition(28, EQUAL, 29)
        .setStateAsFinal(29, Token::Type::GREATER_EQUAL);

    // "-", "->"
    builder.addTransition(0, MINUS, 30)
        .setStateAsFinal(30, Token::Type::MINUS)
        .addTransition(30, GREATER, 31)
        .setStateAsFinal(31, Token::Type::ARROW);

    // !="
    builder.addTransition(0, BANG, 32)
        .addTransition(32, EQUAL, 33)
        .setStateAsFinal(33, Token::Type::BANG_EQUAL);

    // "/", "//", "/* ... */"
    builder.addTransition(0, SLASH, 34)
        .setStateAsFinal(34, Token::Type::SLASH)
        .addTransition(34, SLASH, 35)
        .addComplementaryTransition(35, LINEFEED, 35)
        .setStateAsFinal(35, Token::Type::COMMENT)
        .addTransition(34, STAR, 36)
        .addComplementaryTransition(36, STAR, 36)
        .addTransition(36, STAR, 37)
        .addComplementaryTransition(37, SLASH, 36)
        .addTransition(37, SLASH, 38)
        .setStateAsFinal(38, Token::Type::COMMENT);

    // builtin
    builder.addTransition(0, UNDERSCORE, 39)
        .addTransition(39, UNDERSCORE, 40)
        .addTransition(40, LETTER, 41)
        .addTransition(41, {LETTER, DIGIT, UNDERSCORE}, 41)
        .setStateAsFinal(41, Token::Type::BUILTIN);

    builder.setInitialState(0);

    return builder.build();
}

}  // namespace PArL
