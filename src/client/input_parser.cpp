// std
#include <iostream>
#include <utility>
#include <vector>

// client
#include <input_parser.hpp>

// fmt
#include <fmt/core.h>

namespace client {

input_parser_t::input_parser_t(std::string_view source)
    : m_source(source)
{
}

void input_parser_t::scan_tokens()
{
    while (!is_at_end()) {
        // we are at the beginning of the next lexeme
        m_start = m_current;

        scan_token();
    }
}

std::vector<std::string> input_parser_t::get_tokens()
{
    return std::move(m_tokens);
}

bool input_parser_t::is_at_end()
{
    return m_current >= m_source.length();
}

void input_parser_t::scan_token()
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

char input_parser_t::advance()
{
    return m_source[m_current++];
}

bool input_parser_t::match(char expected)
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

char input_parser_t::peek()
{
    if (is_at_end()) {
        return '\0';
    }

    return m_source[m_current];
}

void input_parser_t::string()
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
    std::string value { m_source,
                        m_start + 1,
                        m_current - 1 - m_start - 1 };

    m_tokens.push_back(value);
}

void input_parser_t::number()
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

    m_tokens.push_back(
        m_source.substr(m_start, m_current - m_start)
    );
}

char input_parser_t::peek_next()
{
    if (m_current + 1 >= m_source.length()) {
        return '\0';
    }

    return m_source[m_current + 1];
}

void input_parser_t::identifier()
{
    while (!is_at_end() && !is_space(peek())) {
        advance();
    }

    std::string value { m_source,
                        m_start,
                        m_current - m_start };

    m_tokens.push_back(value);
}

bool input_parser_t::is_space(char c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\n';
}

bool input_parser_t::is_digit(char c)
{
    return c >= '0' && c <= '9';
}

bool input_parser_t::is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
           || c == '_';
}

bool input_parser_t::is_alpha_numeric(char c)
{
    return is_digit(c) || is_alpha(c);
}

} // namespace client
