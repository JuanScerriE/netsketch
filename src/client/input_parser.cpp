// client
#include "input_parser.hpp"

// std
#include <utility>
#include <vector>

// fmt
#include <fmt/core.h>

namespace client {

InputParser::InputParser(std::string_view source)
    : m_source(source)
{
}

void InputParser::scan_tokens()
{
    while (!is_at_end()) {
        // we are at the beginning of the next lexeme
        m_start = m_current;

        scan_token();
    }
}

std::vector<std::string> InputParser::get_tokens()
{
    return std::move(m_tokens);
}

bool InputParser::is_at_end()
{
    return m_current >= m_source.length();
}

void InputParser::scan_token()
{
    char c = advance();

    switch (c) {
    case ' ':
    case '\r':
    case '\t':
    case '\n':
        // ignore whitespace
        break;
    case '"':
        string();
        break;
    default:
        identifier();
        break;
    }
}

char InputParser::advance()
{
    return m_source[m_current++];
}

bool InputParser::match(char expected)
{
    if (is_at_end()) {
        return false;
    }

    if (m_source[m_current] != expected) {
        return false;
    }

    m_current++;

    return true;
}

char InputParser::peek()
{
    if (is_at_end()) {
        return '\0';
    }

    return m_source[m_current];
}

void InputParser::string()
{
    while (peek() != '"' && !is_at_end()) {
        advance();
    }

    if (is_at_end()) {
        throw std::runtime_error("unterminated string");
    }

    // the closing "
    advance();

    // we can use substring constructor
    std::string value { m_source, m_start + 1, m_current - 1 - m_start - 1 };

    m_tokens.push_back(value);
}

void InputParser::number()
{
    while (is_digit(peek())) {
        advance();
    }

    // look for fractional part
    if (peek() == '.' && is_digit(peek_next())) {
        // consume the "."
        advance();
        while (is_digit(peek())) {
            advance();
        }
    }

    m_tokens.push_back(m_source.substr(m_start, m_current - m_start));
}

char InputParser::peek_next()
{
    if (m_current + 1 >= m_source.length()) {
        return '\0';
    }

    return m_source[m_current + 1];
}

void InputParser::identifier()
{
    while (!is_at_end() && !is_space(peek())) {
        advance();
    }

    std::string value { m_source, m_start, m_current - m_start };

    m_tokens.push_back(value);
}

bool InputParser::is_space(char c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\n';
}

bool InputParser::is_digit(char c)
{
    return c >= '0' && c <= '9';
}

bool InputParser::is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool InputParser::is_alpha_numeric(char c)
{
    return is_digit(c) || is_alpha(c);
}

} // namespace client
