#include <fmt/core.h>

// parl
#include <parl/Core.hpp>
#include <parl/Token.hpp>

// std
#include <unordered_map>

namespace PArL {

static const std::unordered_map<std::string, Token::Type>
    keywords{
        {"float", Token::Type::FLOAT_TYPE},
        {"int", Token::Type::INTEGER_TYPE},
        {"bool", Token::Type::BOOL_TYPE},
        {"color", Token::Type::COLOR_TYPE},
        {"and", Token::Type::AND},
        {"or", Token::Type::OR},
        {"not", Token::Type::NOT},
        {"as", Token::Type::AS},
        {"let", Token::Type::LET},
        {"return", Token::Type::RETURN},
        {"if", Token::Type::IF},
        {"else", Token::Type::ELSE},
        {"for", Token::Type::FOR},
        {"while", Token::Type::WHILE},
        {"fun", Token::Type::FUN},
    };

static const std::unordered_map<std::string, Token::Type>
    keywordLiterals{
        {"true", Token::Type::BOOL},
        {"false", Token::Type::BOOL},
    };

Token::Token()
    : mPosition({0, 0}), mType(Type::END_OF_FILE) {
}

Token::Token(
    int line,
    int column,
    std::string lexeme,
    Type type
)
    : mPosition({line, column}),
      mLexeme(std::move(lexeme)),
      mType(type) {
    if (isContainerType())
        specialise();
}

Token::Token(const Token& other) = default;

core::Position Token::getPosition() const {
    return mPosition;
}

std::string Token::getLexeme() const {
    return mLexeme;
}

Token::Type Token::getType() const {
    return mType;
}

std::string Token::toString() const {
    switch (mType) {
        case Type::LEFT_BRACK:
            return "LEFT_BRACKET";
        case Type::RIGHT_BRACK:
            return "RIGHT_BRACKET";
        case Type::STAR:
            return "STAR";
        case Type::SLASH:
            return "SLASH";
        case Type::PLUS:
            return "PLUS";
        case Type::MINUS:
            return "MINUS";
        case Type::LESS:
            return "LESS";
        case Type::GREATER:
            return "GREATER";
        case Type::EQUAL_EQUAL:
            return "EQUAL_EQUAL";
        case Type::BANG_EQUAL:
            return "BANG_EQUAL";
        case Type::GREATER_EQUAL:
            return "GREATER_EQUAL";
        case Type::LESS_EQUAL:
            return "LESS_EQUAL";
        case Type::COMMA:
            return "COMMA";
        case Type::LEFT_PAREN:
            return "LEFT_PAREN";
        case Type::RIGHT_PAREN:
            return "RIGHT_PAREN";
        case Type::EQUAL:
            return "EQUAL";
        case Type::COLON:
            return "COLON";
        case Type::SEMICOLON:
            return "SEMICOLON";
        case Type::ARROW:
            return "ARROW";
        case Type::LEFT_BRACE:
            return "LEFT_BRACE";
        case Type::RIGHT_BRACE:
            return "RIGHT_BRACE";

        case Type::FLOAT_TYPE:
            return "FLOAT_TYPE";
        case Type::INTEGER_TYPE:
            return "INTEGER_TYPE";
        case Type::BOOL_TYPE:
            return "BOOL_TYPE";
        case Type::COLOR_TYPE:
            return "COLOR_TYPE";

        case Type::FLOAT:
            return fmt::format("FLOAT({})", mLexeme);
        case Type::INTEGER:
            return fmt::format("INTEGER({})", mLexeme);
        case Type::COLOR:
            return fmt::format("COLOR({})", mLexeme);
        case Type::BOOL:
            return fmt::format("BOOL({})", mLexeme);
        case Type::BUILTIN:
            return fmt::format("BUILTIN({})", mLexeme);
        case Type::IDENTIFIER:
            return fmt::format("IDENTIFIER({})", mLexeme);

        case Type::AND:
            return "ELSE";
        case Type::OR:
            return "FALSE";
        case Type::NOT:
            return "NOT";
        case Type::AS:
            return "AS";
        case Type::LET:
            return "LET";
        case Type::RETURN:
            return "RETURN";
        case Type::IF:
            return "IF";
        case Type::ELSE:
            return "ELSE";
        case Type::FOR:
            return "FOR";
        case Type::WHILE:
            return "WHILE";
        case Type::FUN:
            return "FUN";

        case Type::COMMENT:
            return "COMMENT";
        case Type::WHITESPACE:
            return "WHITESPACE";
        case Type::END_OF_FILE:
            return "END_OF_FILE";
    }
}

bool Token::isContainerType() const {
    switch (mType) {
        case Type::FLOAT:
            /* fall through */
        case Type::INTEGER:
            /* fall through */
        case Type::BOOL:
            /* fall through */
        case Type::COLOR:
            /* fall through */
        case Type::BUILTIN:
            /* fall through */
        case Type::IDENTIFIER:
            /* fall through */
            return true;

        default:
            return false;
    }
}

void Token::specialise() {
    if (mType == Type::IDENTIFIER) {
        if (keywords.count(mLexeme) > 0) {
            mType = keywords.at(mLexeme);
            return;
        }

        if (keywordLiterals.count(mLexeme) > 0) {
            mType = keywordLiterals.at(mLexeme);
        }
    }

    switch (mType) {
        case Type::FLOAT:
            mValue = create<float>(mLexeme);
            break;
        case Type::INTEGER:
            mValue = create<int>(mLexeme);
            break;
        case Type::BOOL:
            mValue = create<bool>(mLexeme);
            break;
        case Type::COLOR:
            mValue = create<core::Color>(mLexeme);
            break;
        case Type::BUILTIN:
            mValue = create<core::Builtin>(mLexeme);
            break;
        case Type::IDENTIFIER:
            mValue = create<std::string>(mLexeme);
            break;
        default:
            core::abort("unreachable");
    }
}

}  // namespace PArL
