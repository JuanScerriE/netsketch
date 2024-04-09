#pragma once

// std
#include <initializer_list>
#include <memory>

// parl
#include <lexer/Lexer.hpp>
#include <parl/AST.hpp>
#include <parl/Token.hpp>

// fmt
#include <fmt/core.h>

// definitions
#define LOOKAHEAD (2)

namespace PArL {

class SyncParser : public std::exception {};

class Parser {
   public:
    explicit Parser(Lexer& lexer);

    [[nodiscard]] bool hasError() const;

    void parse(std::string const& source);

    std::unique_ptr<core::Program> getAst();

    void reset();

   private:
    std::unique_ptr<core::Type> type();

    std::unique_ptr<core::Program> program();
    std::unique_ptr<core::Stmt> statement();
    std::unique_ptr<core::Block> block();
    std::unique_ptr<core::VariableDecl> variableDecl();
    std::unique_ptr<core::Assignment> assignment();
    std::unique_ptr<core::PrintStmt> printStatement();
    std::unique_ptr<core::DelayStmt> delayStatement();
    std::unique_ptr<core::WriteBoxStmt> writeBoxStatement();
    std::unique_ptr<core::WriteStmt> writeStatement();
    std::unique_ptr<core::ClearStmt> clearStatement();
    std::unique_ptr<core::IfStmt> ifStmt();
    std::unique_ptr<core::ForStmt> forStmt();
    std::unique_ptr<core::WhileStmt> whileStmt();
    std::unique_ptr<core::ReturnStmt> returnStmt();
    std::unique_ptr<core::FunctionDecl> functionDecl();
    std::unique_ptr<core::FormalParam> formalParam();

    std::unique_ptr<core::PadWidth> padWidth();
    std::unique_ptr<core::PadHeight> padHeight();
    std::unique_ptr<core::PadRead> padRead();
    std::unique_ptr<core::PadRandomInt> padRandomInt();
    std::unique_ptr<core::BooleanLiteral> booleanLiteral();
    std::unique_ptr<core::ColorLiteral> colorLiteral();
    std::unique_ptr<core::FloatLiteral> floatLiteral();
    std::unique_ptr<core::IntegerLiteral> integerLiteral();
    std::unique_ptr<core::ArrayLiteral> arrayLiteral();
    std::unique_ptr<core::SubExpr> subExpr();
    std::unique_ptr<core::Variable> variable();
    std::unique_ptr<core::ArrayAccess> arrayAccess();
    std::unique_ptr<core::FunctionCall> functionCall();

    std::unique_ptr<core::Expr> expr();
    std::unique_ptr<core::Expr> logicOr();
    std::unique_ptr<core::Expr> logicAnd();
    std::unique_ptr<core::Expr> equality();
    std::unique_ptr<core::Expr> comparison();
    std::unique_ptr<core::Expr> term();
    std::unique_ptr<core::Expr> factor();
    std::unique_ptr<core::Expr> unary();
    std::unique_ptr<core::Expr> primary();

    void initWindow();
    void moveWindow();

    Token nextToken();
    Token peek();
    Token peek(int lookahead);
    Token advance();
    Token previous();

    bool isAtEnd();
    bool check(Token::Type type);
    bool peekMatch(
        std::initializer_list<Token::Type> const& types
    );
    bool match(
        std::initializer_list<Token::Type> const& types
    );

    template <typename... T>
    void consume(
        Token::Type type,
        fmt::format_string<T...> fmt,
        T&&... args
    ) {
        if (check(type)) {
            advance();
        } else {
            error(fmt, args...);
        }
    }

    template <typename... T>
    void error(fmt::format_string<T...> fmt, T&&... args) {
        mHasError = true;

        Token violatingToken = peek();

        fmt::println(
            stderr,
            "parsing error at {}:{}:: {}",
            violatingToken.getPosition().row(),
            violatingToken.getPosition().col(),
            fmt::format(fmt, args...)
        );

        throw SyncParser{};
    }

    void synchronize();

    static std::optional<core::Base> primitiveFromToken(
        Token& token
    );
    static std::optional<core::Operation>
    operationFromToken(Token& token);

    Lexer& mLexer;
    bool mHasError{false};
    std::unique_ptr<core::Program> mAst{};
    Token mPreviousToken;
    std::array<Token, LOOKAHEAD> mTokenBuffer;
};

}  // namespace PArL
