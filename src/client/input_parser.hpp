#pragma once

// std
#include <string>
#include <vector>

namespace client {

// The input parser has been stripped from the
// lexer I had written for the Lox toy programming
// language.

class InputParser {
   public:
    explicit InputParser(std::string_view source);

    void scan_tokens();

    std::vector<std::string> get_tokens();

   private:
    bool is_at_end();
    void scan_token();
    char advance();
    bool match(char expected);
    char peek();
    void string();
    void number();
    char peek_next();
    void identifier();

    static bool is_space(char c);
    static bool is_digit(char c);
    static bool is_alpha(char c);
    static bool is_alpha_numeric(char c);

    const std::string m_source;

    std::vector<std::string> m_tokens {};

    int mLine = 1;
    size_t m_current = 0;
    size_t m_start = 0;
};

} // namespace client
