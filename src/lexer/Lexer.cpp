// fmt
#include <fmt/core.h>

// std
#include <stack>
#include <utility>

// parl
#include <lexer/Dfsa.hpp>
#include <lexer/Lexer.hpp>
#include <parl/Core.hpp>
#include <parl/Errors.hpp>

namespace PArL {

Lexer::Lexer(
    Dfsa dfsa,
    std::unordered_map<int, std::function<bool(char)>>
        categoryToChecker,
    std::unordered_map<int, Token::Type>
        finalStateToTokenType
)
    : mDfsa(std::move(dfsa)),
      mCategoryToChecker(std::move(categoryToChecker)),
      mFinalStateToTokenType(std::move(finalStateToTokenType
      )) {
}

void Lexer::reset() {
    mCursor = 0;
    mLine = 1;
    mColumn = 1;
    mHasError = false;
    mSource.clear();
}

void Lexer::addSource(std::string const& source) {
    reset();

    mSource = source;
}

std::optional<Token> Lexer::nextToken() {
    if (isAtEnd(0))
        return Token{
            mLine,
            mColumn,
            "",
            Token::Type::END_OF_FILE
        };

    auto [state, lexeme] = simulateDFSA();

    std::optional<Token> token{};

    if (state == INVALID_STATE) {
        mHasError = true;

        fmt::println(
            stderr,
            "lexical error at {}:{}:: unexpected "
            "lexeme '{}'",
            mLine,
            mColumn,
            lexeme
        );
    } else {
        try {
            token = createToken(
                lexeme,
                mFinalStateToTokenType.at(state)
            );
        } catch (UndefinedBuiltin& error) {
            mHasError = true;

            fmt::println(
                stderr,
                "lexical error at {}:{}:: {}",
                mLine,
                mColumn,
                error.what()
            );
        }
    }

    updateLocationState(lexeme);

    return token;
}

Dfsa const& Lexer::getDfsa() const {
    return mDfsa;
}

bool Lexer::hasError() const {
    return mHasError;
}

Token Lexer::createToken(
    std::string const& lexeme,
    Token::Type type
) const {
    return Token{mLine, mColumn, lexeme, type};
}

bool Lexer::isAtEnd(size_t offset) const {
    return mCursor + offset >= mSource.length();
}

void Lexer::updateLocationState(std::string const& lexeme) {
    for (char ch : lexeme) {
        mCursor++;

        if (ch == '\n') {
            mLine++;

            mColumn = 1;
        } else {
            mColumn++;
        }
    }
}

std::optional<char> Lexer::nextCharacter(size_t cursor
) const {
    if (!isAtEnd(cursor))
        return mSource[mCursor + cursor];

    return {};
}

std::vector<int> Lexer::categoriesOf(char character) const {
    std::vector<int> satisfiedCategories{};

    for (auto& [category, check] : mCategoryToChecker) {
        if (check(character))
            satisfiedCategories.emplace_back(category);
    }

    return satisfiedCategories;
}

std::pair<int, std::string> Lexer::simulateDFSA() {
    int state = mDfsa.getInitialState();

    size_t lCursor = 0;  // local cursor

    std::stack<int> stack;

    std::string lexeme;

    for (;;) {
        if (mDfsa.isFinalState(state)) {
            while (!stack.empty()) {
                stack.pop();
            }
        }

        stack.push(state);

        std::optional<char> ch = nextCharacter(lCursor);

        if (!ch.has_value())
            break;  // end of file

        std::vector<int> categories =
            categoriesOf(ch.value());

        state = !categories.empty()
                    ? mDfsa.getTransition(state, categories)
                    : INVALID_STATE;

        if (state == INVALID_STATE)
            break;  // no more transitions are available

        lCursor++;

        lexeme += ch.value();
    }

    core::abort_if(
        stack.empty(),
        "stack should never be emtpy"
    );

    for (;;) {
        state = stack.top();

        stack.pop();

        if (mDfsa.isFinalState(state))
            return {state, lexeme};

        if (stack.empty())
            break;

        lexeme.pop_back();
    }

    lexeme = mSource.substr(mCursor, lCursor + 1);

    return {INVALID_STATE, lexeme};
}

}  // namespace PArL
