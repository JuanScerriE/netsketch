#pragma once

// parl
#include <lexer/LexerBuilder.hpp>
#include <parl/Token.hpp>

namespace PArL {

class LexerDirector {
   public:
    static Lexer buildLexer();
};

}  // namespace PArL
