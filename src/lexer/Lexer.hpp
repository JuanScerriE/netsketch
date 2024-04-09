#pragma once

// std
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

// parl
#include <lexer/Dfsa.hpp>
#include <parl/Token.hpp>

namespace PArL {

class Lexer {
   public:
    Lexer(
        Dfsa dfsa,
        std::unordered_map<int, std::function<bool(char)>>
            categoryToChecker,
        std::unordered_map<int, Token::Type>
            finalStateToTokenType
    );

    void reset();

    void addSource(std::string const& source);

    std::optional<Token> nextToken();

    [[nodiscard]] Dfsa const& getDfsa() const;

    bool hasError() const;

   private:
    [[nodiscard]] Token createToken(
        std::string const& lexeme,
        Token::Type type
    ) const;

    bool isAtEnd(size_t offset) const;
    void updateLocationState(std::string const& lexeme);

    [[nodiscard]] std::optional<char> nextCharacter(
        size_t cursor
    ) const;
    [[nodiscard]] std::vector<int> categoriesOf(
        char character
    ) const;
    [[nodiscard]] std::pair<int, std::string> simulateDFSA(
    );

    // source info
    size_t mCursor = 0;

    int mLine = 1;
    int mColumn = 1;

    std::string mSource{};

    // error info
    bool mHasError = false;

    // dfsa
    const Dfsa mDfsa;

    // category checkers
    const std::unordered_map<int, std::function<bool(char)>>
        mCategoryToChecker;

    // final state to token type association
    const std::unordered_map<int, Token::Type>
        mFinalStateToTokenType;
};

}  // namespace PArL
