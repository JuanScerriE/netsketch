// fmt
#include <fmt/core.h>

// std
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>

// parl
#include <lexer/Lexer.hpp>
#include <parl/AST.hpp>
#include <parser/Parser.hpp>

namespace PArL {

// HACK: this is technically a hack to avoid
// rewriting the constructors of all the
// AST nodes to keep track of a position
template <typename T, typename... Args>
static inline std::unique_ptr<T>
make_with_pos(core::Position pos, Args &&...args) {
    static_assert(
        std::is_base_of_v<core::Node, T>,
        "type T does inherit from type Node"
    );

    auto unique_ptr =
        std::make_unique<T>(std::forward<Args>(args)...);

    unique_ptr->position = pos;

    return unique_ptr;
}

Parser::Parser(Lexer &lexer)
    : mLexer(lexer) {
    initWindow();
}

void Parser::parse(std::string const &source) {
    mLexer.addSource(source);
    reset();
    mAst = program();
}

bool Parser::hasError() const {
    return mHasError;
}

void Parser::reset() {
    mHasError = false;
    mAst.reset();
    initWindow();
}

std::unique_ptr<core::Program> Parser::getAst() {
    core::abort_if(
        mHasError,
        "parser could not generate tree due to parsing "
        "error(s)"
    );

    return std::move(mAst);
}

std::unique_ptr<core::Program> Parser::program() {
    std::vector<std::unique_ptr<core::Stmt>> stmts;

    while (!isAtEnd()) {
        try {
            stmts.emplace_back(statement());
        } catch (SyncParser const &) {
            synchronize();
        }
    }

    return make_with_pos<core::Program>(
        {1, 1},
        std::move(stmts)
    );
}

std::unique_ptr<core::Stmt> Parser::statement() {
    Token peekToken = peek();

    switch (peekToken.getType()) {
        case Token::Type::BUILTIN: {
            auto builtinType =
                *peekToken.asOpt<core::Builtin>();

            switch (builtinType) {
                case core::Builtin::PRINT: {
                    std::unique_ptr<core::PrintStmt> stmt =
                        printStatement();
                    consume(
                        Token::Type::SEMICOLON,
                        "expected ';' after __print "
                        "statement"
                    );
                    return stmt;
                }
                case core::Builtin::DELAY: {
                    std::unique_ptr<core::DelayStmt> stmt =
                        delayStatement();
                    consume(
                        Token::Type::SEMICOLON,
                        "expected ';' after __delay "
                        "statement"
                    );
                    return stmt;
                }
                case core::Builtin::WRITE: {
                    std::unique_ptr<core::WriteStmt> stmt =
                        writeStatement();
                    consume(
                        Token::Type::SEMICOLON,
                        "expected ';' after __write "
                        "statement"
                    );
                    return stmt;
                }
                case core::Builtin::CLEAR: {
                    std::unique_ptr<core::ClearStmt> stmt =
                        clearStatement();
                    consume(
                        Token::Type::SEMICOLON,
                        "expected ';' after __clear "
                        "statement"
                    );
                    return stmt;
                }
                case core::Builtin::WRITE_BOX: {
                    std::unique_ptr<core::WriteBoxStmt>
                        stmt = writeBoxStatement();
                    consume(
                        Token::Type::SEMICOLON,
                        "expected ';' after "
                        "__write_box statement"
                    );
                    return stmt;
                }
                default:
                    error(
                        "unexpected token {} for "
                        "builtin "
                        "statement",
                        peekToken.toString()
                    );
            }
        } break;
        case Token::Type::LEFT_BRACE:
            return block();
        case Token::Type::IF:
            return ifStmt();
        case Token::Type::FOR:
            return forStmt();
        case Token::Type::WHILE:
            return whileStmt();
        case Token::Type::FUN:
            return functionDecl();
        case Token::Type::RETURN: {
            std::unique_ptr<core::ReturnStmt> stmt =
                returnStmt();
            consume(
                Token::Type::SEMICOLON,
                "expected ';' after "
                "return statement"
            );
            return stmt;
        }
        case Token::Type::LET: {
            std::unique_ptr<core::VariableDecl> stmt =
                variableDecl();
            consume(
                Token::Type::SEMICOLON,
                "expected ';' after "
                "variable declaration"
            );
            return stmt;
        }
        case Token::Type::IDENTIFIER: {
            std::unique_ptr<core::Assignment> stmt =
                assignment();
            consume(
                Token::Type::SEMICOLON,
                "expected ';' after "
                "assignment"
            );
            return stmt;
        }
        default:
            error(
                "unexpected token {} for "
                "statement start",
                peekToken.toString()
            );
    }
}

std::unique_ptr<core::Block> Parser::block() {
    consume(
        Token::Type::LEFT_BRACE,
        "expected '{{' at start of block"
    );

    std::vector<std::unique_ptr<core::Stmt>> stmts;

    while (!isAtEnd() &&
           !peekMatch({Token::Type::RIGHT_BRACE})) {
        try {
            stmts.emplace_back(statement());
        } catch (SyncParser const &) {
            synchronize();
        }
    }

    consume(
        Token::Type::RIGHT_BRACE,
        "expected '}}' at end of block"
    );

    Token rightBrace = previous();

    return make_with_pos<core::Block>(
        rightBrace.getPosition(),
        std::move(stmts)
    );
}

std::unique_ptr<core::VariableDecl> Parser::variableDecl() {
    consume(
        Token::Type::LET,
        "expected 'let' at the start of variable "
        "declaration"
    );

    consume(
        Token::Type::IDENTIFIER,
        "expected identifier token "
        "instead received {}",
        peek().toString()
    );

    Token token = previous();

    consume(
        Token::Type::COLON,
        "expected ':' after identifier"
    );

    std::unique_ptr<core::Type> type_ = type();

    consume(Token::Type::EQUAL, "expected '=' after type");

    std::unique_ptr<core::Expr> expression = expr();

    return make_with_pos<core::VariableDecl>(
        token.getPosition(),
        token.getLexeme(),
        std::move(type_),
        std::move(expression)
    );
}

std::unique_ptr<core::Assignment> Parser::assignment() {
    consume(
        Token::Type::IDENTIFIER,
        "expected identifier token "
        "instead received {}",
        peek().toString()
    );

    Token token = previous();

    std::unique_ptr<core::Expr> index{};

    if (match({Token::Type::LEFT_BRACK})) {
        index = expr();

        consume(
            Token::Type::RIGHT_BRACK,
            "expected ']' after integer literal"
        );
    }

    consume(
        Token::Type::EQUAL,
        "expected '=' after identifier"
    );

    std::unique_ptr<core::Expr> expr_ = expr();

    return make_with_pos<core::Assignment>(
        token.getPosition(),
        token.getLexeme(),
        std::move(index),
        std::move(expr_)
    );
}

std::unique_ptr<core::PrintStmt> Parser::printStatement() {
    consume(Token::Type::BUILTIN, "expected __print");

    Token print = previous();

    std::unique_ptr<core::Expr> expr_ = expr();

    return make_with_pos<core::PrintStmt>(
        print.getPosition(),
        std::move(expr_)
    );
}

std::unique_ptr<core::DelayStmt> Parser::delayStatement() {
    consume(Token::Type::BUILTIN, "expected __delay");

    Token token = previous();

    std::unique_ptr<core::Expr> expr_ = expr();

    return make_with_pos<core::DelayStmt>(
        token.getPosition(),
        std::move(expr_)
    );
}

std::unique_ptr<core::WriteStmt> Parser::writeStatement() {
    consume(Token::Type::BUILTIN, "expected __write");

    Token token = previous();

    std::unique_ptr<core::Expr> x = expr();

    consume(
        Token::Type::COMMA,
        "expected ',' after expression"
    );

    std::unique_ptr<core::Expr> y = expr();

    consume(
        Token::Type::COMMA,
        "expected ',' after expression"
    );

    std::unique_ptr<core::Expr> color = expr();

    return make_with_pos<core::WriteStmt>(
        token.getPosition(),
        std::move(x),
        std::move(y),
        std::move(color)
    );
}

std::unique_ptr<core::ClearStmt> Parser::clearStatement() {
    consume(Token::Type::BUILTIN, "expected __clear");

    Token token = previous();

    std::unique_ptr<core::Expr> color = expr();

    return make_with_pos<core::ClearStmt>(
        token.getPosition(),
        std::move(color)
    );
}

std::unique_ptr<core::WriteBoxStmt>
Parser::writeBoxStatement() {
    consume(Token::Type::BUILTIN, "expected __write_box");

    Token token = previous();

    std::unique_ptr<core::Expr> x = expr();

    consume(
        Token::Type::COMMA,
        "expected ',' after expression"
    );

    std::unique_ptr<core::Expr> y = expr();

    consume(
        Token::Type::COMMA,
        "expected ',' after expression"
    );

    std::unique_ptr<core::Expr> xOffset = expr();

    consume(
        Token::Type::COMMA,
        "expected ',' after expression"
    );

    std::unique_ptr<core::Expr> yOffset = expr();

    consume(
        Token::Type::COMMA,
        "expected ',' after expression"
    );

    std::unique_ptr<core::Expr> color = expr();

    return make_with_pos<core::WriteBoxStmt>(
        token.getPosition(),
        std::move(x),
        std::move(y),
        std::move(xOffset),
        std::move(yOffset),
        std::move(color)
    );
}

std::unique_ptr<core::IfStmt> Parser::ifStmt() {
    consume(
        Token::Type::IF,
        "expected 'if' at start of if statement"
    );

    Token token = previous();

    consume(
        Token::Type::LEFT_PAREN,
        "expected '(' after 'if'"
    );

    std::unique_ptr<core::Expr> cond = expr();

    consume(
        Token::Type::RIGHT_PAREN,
        "expected ')' after expression"
    );

    std::unique_ptr<core::Block> thenBlock = block();

    std::unique_ptr<core::Block> elseBlock{};

    if (match({Token::Type::ELSE})) {
        elseBlock = block();
    }

    return make_with_pos<core::IfStmt>(
        token.getPosition(),
        std::move(cond),
        std::move(thenBlock),
        std::move(elseBlock)
    );
}

std::unique_ptr<core::ForStmt> Parser::forStmt() {
    consume(
        Token::Type::FOR,
        "expected 'for' at start of for statement"
    );

    Token token = previous();

    consume(
        Token::Type::LEFT_PAREN,
        "expected '(' after 'for'"
    );

    std::unique_ptr<core::VariableDecl> decl{};

    if (!peekMatch({Token::Type::SEMICOLON})) {
        decl = variableDecl();
    }

    consume(
        Token::Type::SEMICOLON,
        "expected ';' after '(' or variable "
        "declaration"
    );

    std::unique_ptr<core::Expr> cond = expr();

    consume(
        Token::Type::SEMICOLON,
        "expected ';' after expression"
    );

    std::unique_ptr<core::Assignment> assign{};

    if (!peekMatch({Token::Type::RIGHT_PAREN})) {
        assign = assignment();
    }

    consume(
        Token::Type::RIGHT_PAREN,
        "expected ')' after ';' or assignment"
    );

    std::unique_ptr<core::Block> block_ = block();

    return make_with_pos<core::ForStmt>(
        token.getPosition(),
        std::move(decl),
        std::move(cond),
        std::move(assign),
        std::move(block_)
    );
}

std::unique_ptr<core::WhileStmt> Parser::whileStmt() {
    consume(
        Token::Type::WHILE,
        "expected 'while' at start of while statement"
    );

    Token token = previous();

    consume(
        Token::Type::LEFT_PAREN,
        "expected '(' after 'while'"
    );

    std::unique_ptr<core::Expr> cond = expr();

    consume(
        Token::Type::RIGHT_PAREN,
        "expected ')' after expression"
    );

    std::unique_ptr<core::Block> block_ = block();

    return make_with_pos<core::WhileStmt>(
        token.getPosition(),
        std::move(cond),
        std::move(block_)
    );
}

std::unique_ptr<core::ReturnStmt> Parser::returnStmt() {
    consume(
        Token::Type::RETURN,
        "expected 'return' at start of return "
        "statement"
    );

    Token token = previous();

    std::unique_ptr<core::Expr> expr_ = expr();

    return make_with_pos<core::ReturnStmt>(
        token.getPosition(),
        std::move(expr_)
    );
}

std::unique_ptr<core::FormalParam> Parser::formalParam() {
    consume(
        Token::Type::IDENTIFIER,
        "expected identifier token "
        "instead received {}",
        peek().toString()
    );

    Token token = previous();

    consume(
        Token::Type::COLON,
        "expected ':' after identifier"
    );

    std::unique_ptr<core::Type> type_ = type();

    return make_with_pos<core::FormalParam>(
        token.getPosition(),
        token.getLexeme(),
        std::move(type_)
    );
}

std::unique_ptr<core::FunctionDecl> Parser::functionDecl() {
    consume(
        Token::Type::FUN,
        "expected 'fun' at start of function "
        "declaration"
    );

    consume(
        Token::Type::IDENTIFIER,
        "expected identifier token "
        "instead received {}",
        peek().toString()
    );

    Token token = previous();

    consume(
        Token::Type::LEFT_PAREN,
        "expected '(' after identifier"
    );

    std::vector<std::unique_ptr<core::FormalParam>>
        formalParams{};

    if (!peekMatch({Token::Type::RIGHT_PAREN})) {
        do {
            formalParams.emplace_back(formalParam());
        } while (match({Token::Type::COMMA}));
    }

    consume(
        Token::Type::RIGHT_PAREN,
        "expected ')' after formal parameters"
    );

    consume(Token::Type::ARROW, "Expected '->' after ')'");

    std::unique_ptr<core::Type> type_ = type();

    std::unique_ptr<core::Block> block_ = block();

    return make_with_pos<core::FunctionDecl>(
        token.getPosition(),
        token.getLexeme(),
        std::move(formalParams),
        std::move(type_),
        std::move(block_)
    );
}

std::unique_ptr<core::Type> Parser::type() {
    Token token = advance();

    std::optional<core::Base> primitive =
        primitiveFromToken(token);

    if (!primitive.has_value()) {
        error(
            "expected type token instead received {}",
            token.toString()
        );
    }

    if (!match({Token::Type::LEFT_BRACK})) {
        return make_with_pos<core::Type>(
            token.getPosition(),
            *primitive,
            false,
            nullptr
        );
    }

    // NOTE: this essentially removes our ability to have
    // things like int[] as types and then type inference
    // does its magic. This is because we are using
    // a primitive type which does not allow non-sized
    // arrays, all arrays must have a size. Hence, we'd have
    // to add another layer to allow for non-sized array
    // in the syntax and then desugar into a sized. But to
    // do this properly we need to actually separate type
    // inference into its own dedicated phase.
    std::unique_ptr<core::IntegerLiteral> integer =
        integerLiteral();

    consume(
        Token::Type::RIGHT_BRACK,
        "expected ']' after integer literal"
    );

    return make_with_pos<core::Type>(
        token.getPosition(),
        *primitive,
        true,
        std::move(integer)
    );
}

std::unique_ptr<core::Expr> Parser::expr() {
    std::unique_ptr<core::Expr> expr = logicOr();

    if (match({Token::Type::AS}))
        expr->type = type();

    return expr;
}

std::unique_ptr<core::Expr> Parser::logicOr() {
    std::unique_ptr<core::Expr> expr = logicAnd();

    while (match({Token::Type::OR})) {
        Token op = previous();

        std::unique_ptr<core::Expr> right = logicAnd();

        expr = make_with_pos<core::Binary>(
            op.getPosition(),
            std::move(expr),
            *operationFromToken(op),
            std::move(right)
        );
    }

    return expr;
}

std::unique_ptr<core::Expr> Parser::logicAnd() {
    std::unique_ptr<core::Expr> expr = equality();

    while (match({Token::Type::AND})) {
        Token op = previous();

        std::unique_ptr<core::Expr> right = equality();

        expr = make_with_pos<core::Binary>(
            op.getPosition(),
            std::move(expr),
            *operationFromToken(op),
            std::move(right)
        );
    }

    return expr;
}

std::unique_ptr<core::Expr> Parser::equality() {
    std::unique_ptr<core::Expr> expr = comparison();

    while (match(
        {Token::Type::EQUAL_EQUAL, Token::Type::BANG_EQUAL}
    )) {
        Token op = previous();

        std::unique_ptr<core::Expr> right = equality();

        expr = make_with_pos<core::Binary>(
            op.getPosition(),
            std::move(expr),
            *operationFromToken(op),
            std::move(right)
        );
    }

    return expr;
}

std::unique_ptr<core::Expr> Parser::comparison() {
    std::unique_ptr<core::Expr> expr = term();

    while (match(
        {Token::Type::LESS,
         Token::Type::LESS_EQUAL,
         Token::Type::GREATER,
         Token::Type::GREATER_EQUAL}
    )) {
        Token op = previous();

        std::unique_ptr<core::Expr> right = term();

        expr = make_with_pos<core::Binary>(
            op.getPosition(),
            std::move(expr),
            *operationFromToken(op),
            std::move(right)
        );
    }

    return expr;
}

std::unique_ptr<core::Expr> Parser::term() {
    std::unique_ptr<core::Expr> expr = factor();

    while (match({Token::Type::PLUS, Token::Type::MINUS})) {
        Token op = previous();

        std::unique_ptr<core::Expr> right = factor();

        expr = make_with_pos<core::Binary>(
            op.getPosition(),
            std::move(expr),
            *operationFromToken(op),
            std::move(right)
        );
    }

    return expr;
}

std::unique_ptr<core::Expr> Parser::factor() {
    std::unique_ptr<core::Expr> expr = unary();

    while (match({Token::Type::STAR, Token::Type::SLASH})) {
        Token op = previous();

        std::unique_ptr<core::Expr> right = unary();

        expr = make_with_pos<core::Binary>(
            op.getPosition(),
            std::move(expr),
            *operationFromToken(op),
            std::move(right)
        );
    }

    return expr;
}

std::unique_ptr<core::Expr> Parser::unary() {
    if (match({Token::Type::MINUS, Token::Type::NOT})) {
        Token op = previous();

        std::unique_ptr<core::Expr> expr = unary();

        return make_with_pos<core::Unary>(
            op.getPosition(),
            *operationFromToken(op),
            std::move(expr)
        );
    }

    return primary();
}

std::unique_ptr<core::Expr> Parser::primary() {
    Token peekToken = peek();

    switch (peekToken.getType()) {
        case Token::Type::BUILTIN: {
            auto builtinType =
                *peekToken.asOpt<core::Builtin>();

            switch (builtinType) {
                case core::Builtin::WIDTH:
                    return padWidth();
                case core::Builtin::HEIGHT:
                    return padHeight();
                case core::Builtin::READ:
                    return padRead();
                case core::Builtin::RANDOM_INT:
                    return padRandomInt();
                default:
                    error(
                        "unexpected token {} for "
                        "builtin "
                        "statement",
                        peekToken.toString()
                    );
            }
        } break;
        case Token::Type::BOOL:
            return booleanLiteral();
        case Token::Type::COLOR:
            return colorLiteral();
        case Token::Type::FLOAT:
            return floatLiteral();
        case Token::Type::INTEGER:
            return integerLiteral();
        case Token::Type::LEFT_BRACK:
            return arrayLiteral();
        case Token::Type::LEFT_PAREN:
            return subExpr();
        case Token::Type::IDENTIFIER: {
            switch (peek(1).getType()) {
                case Token::Type::LEFT_PAREN:
                    return functionCall();
                case Token::Type::LEFT_BRACK:
                    return arrayAccess();
                default:
                    return variable();
            }
        }
        default: {
            error(
                "unexpected token {} for primary",
                peekToken.toString()
            );
        }
    }
}

std::unique_ptr<core::BooleanLiteral>
Parser::booleanLiteral() {
    consume(Token::Type::BOOL, "expected boolean literal");

    Token token = previous();

    return make_with_pos<core::BooleanLiteral>(
        token.getPosition(),
        *token.asOpt<bool>()
    );
}

std::unique_ptr<core::ColorLiteral> Parser::colorLiteral() {
    consume(Token::Type::COLOR, "expected color literal");

    Token token = previous();

    return make_with_pos<core::ColorLiteral>(
        token.getPosition(),
        *token.asOpt<core::Color>()
    );
}

std::unique_ptr<core::FloatLiteral> Parser::floatLiteral() {
    consume(Token::Type::FLOAT, "expected float literal");

    Token token = previous();

    return make_with_pos<core::FloatLiteral>(
        token.getPosition(),
        *token.asOpt<float>()
    );
}

std::unique_ptr<core::IntegerLiteral>
Parser::integerLiteral() {
    consume(
        Token::Type::INTEGER,
        "expected integer literal"
    );

    Token token = previous();

    return make_with_pos<core::IntegerLiteral>(
        token.getPosition(),
        *token.asOpt<int>()
    );
}

std::unique_ptr<core::ArrayLiteral> Parser::arrayLiteral() {
    consume(
        Token::Type::LEFT_BRACK,
        "expected '[' at start of array literal"
    );

    core::Position position = previous().getPosition();

    std::vector<std::unique_ptr<core::Expr>> exprs{};

    if (!peekMatch({Token::Type::RIGHT_BRACK})) {
        do {
            exprs.emplace_back(expr());
        } while (match({Token::Type::COMMA}));
    }

    consume(
        Token::Type::RIGHT_BRACK,
        "expected ']' at end of array literal"
    );

    return make_with_pos<core::ArrayLiteral>(
        position,
        std::move(exprs)
    );
}

std::unique_ptr<core::PadWidth> Parser::padWidth() {
    consume(Token::Type::BUILTIN, "expected __width");

    return make_with_pos<core::PadWidth>(
        previous().getPosition()
    );
}

std::unique_ptr<core::PadHeight> Parser::padHeight() {
    consume(Token::Type::BUILTIN, "expected __height");

    return make_with_pos<core::PadHeight>(
        previous().getPosition()
    );
}

std::unique_ptr<core::PadRead> Parser::padRead() {
    consume(Token::Type::BUILTIN, "expected __read");

    core::Position position = previous().getPosition();

    std::unique_ptr<core::Expr> x = expr();

    consume(
        Token::Type::COMMA,
        "expected ',' after expression"
    );

    std::unique_ptr<core::Expr> y = expr();

    return make_with_pos<core::PadRead>(
        position,
        std::move(x),
        std::move(y)
    );
}

std::unique_ptr<core::PadRandomInt> Parser::padRandomInt() {
    consume(Token::Type::BUILTIN, "expected __random_int");

    core::Position position = previous().getPosition();

    std::unique_ptr<core::Expr> max = expr();

    return make_with_pos<core::PadRandomInt>(
        position,
        std::move(max)
    );
}

std::unique_ptr<core::SubExpr> Parser::subExpr() {
    consume(
        Token::Type::LEFT_PAREN,
        "expected '(' at start of sub expression"
    );

    Token leftParen = previous();

    std::unique_ptr<core::Expr> expr_{expr()};

    consume(
        Token::Type::RIGHT_PAREN,
        "expected ')' at end of sub expression"
    );

    return make_with_pos<core::SubExpr>(
        leftParen.getPosition(),
        std::move(expr_)
    );
}

std::unique_ptr<core::Variable> Parser::variable() {
    consume(
        Token::Type::IDENTIFIER,
        "expected identifier token "
        "instead received {}",
        peek().toString()
    );

    Token token = previous();

    return make_with_pos<core::Variable>(
        token.getPosition(),
        token.getLexeme()
    );
}

std::unique_ptr<core::ArrayAccess> Parser::arrayAccess() {
    consume(
        Token::Type::IDENTIFIER,
        "expected identifier token "
        "instead received {}",
        peek().toString()
    );

    Token token = previous();

    consume(
        Token::Type::LEFT_BRACK,
        "expected '[' after identifier"
    );

    std::unique_ptr<core::Expr> expr_{expr()};

    consume(
        Token::Type::RIGHT_BRACK,
        "expected ']' after expression"
    );

    return make_with_pos<core::ArrayAccess>(
        token.getPosition(),
        token.getLexeme(),
        std::move(expr_)
    );
}

std::unique_ptr<core::FunctionCall> Parser::functionCall() {
    consume(
        Token::Type::IDENTIFIER,
        "expected identifier token "
        "instead received {}",
        peek().toString()
    );

    Token token = previous();

    consume(
        Token::Type::LEFT_PAREN,
        "expected '(' after identifier"
    );

    std::vector<std::unique_ptr<core::Expr>> params{};

    if (!peekMatch({Token::Type::RIGHT_PAREN})) {
        do {
            params.emplace_back(expr());
        } while (match({Token::Type::COMMA}));
    }

    consume(
        Token::Type::RIGHT_PAREN,
        "expected ')' after parameters"
    );

    return make_with_pos<core::FunctionCall>(
        token.getPosition(),
        token.getLexeme(),
        std::move(params)
    );
}

void Parser::initWindow() {
    for (int i = 0; i < LOOKAHEAD; i++) {
        mTokenBuffer[i] = nextToken();
    }

    mPreviousToken = mTokenBuffer[0];
}

void Parser::moveWindow() {
    mPreviousToken = mTokenBuffer[0];

    for (int i = 1; i < LOOKAHEAD; i++) {
        mTokenBuffer[i - 1] = mTokenBuffer[i];
    }

    mTokenBuffer[LOOKAHEAD - 1] = nextToken();
}

Token Parser::nextToken() {
    std::optional<Token> token;

    do {
        token = mLexer.nextToken();
    } while (!token.has_value() ||
             token->getType() == Token::Type::WHITESPACE ||
             token->getType() == Token::Type::COMMENT);

    return *token;
}

Token Parser::peek() {
    return peek(0);
}

Token Parser::peek(int lookahead) {
    core::abort_if(
        !(0 <= lookahead && lookahead < LOOKAHEAD),
        "exceeded lookahead of {}",
        LOOKAHEAD
    );

    return mTokenBuffer[lookahead];
}

Token Parser::advance() {
    moveWindow();

    return mPreviousToken;
}

Token Parser::previous() {
    return mPreviousToken;
}

bool Parser::isAtEnd() {
    return peek().getType() == Token::Type::END_OF_FILE;
}

bool Parser::check(Token::Type type) {
    if (isAtEnd()) {
        return false;
    }

    return peek().getType() == type;
}

bool Parser::peekMatch(
    std::initializer_list<Token::Type> const &types
) {
    return std::any_of(
        types.begin(),
        types.end(),
        [&](auto type) {
            return check(type);
        }
    );
}

bool Parser::match(
    std::initializer_list<Token::Type> const &types
) {
    if (peekMatch(types)) {
        advance();

        return true;
    }

    return false;
}

// NOTE: synchronization is all best effort

void Parser::synchronize() {
    while (!isAtEnd()) {
        Token peekToken = peek();

        switch (peekToken.getType()) {
            case Token::Type::SEMICOLON:
                advance();

                return;
            case Token::Type::FOR:
                /* fall through */
            case Token::Type::FUN:
                /* fall through */
            case Token::Type::IF:
                /* fall through */
            case Token::Type::LET:
                /* fall through */
            case Token::Type::RETURN:
                /* fall through */
            case Token::Type::WHILE:
                /* fall through */
            case Token::Type::LEFT_BRACE:
                /* fall through */
                return;

            case Token::Type::BUILTIN: {
                auto builtinType =
                    *peekToken.asOpt<core::Builtin>();

                switch (builtinType) {
                    case core::Builtin::PRINT:
                        /* fall through */
                    case core::Builtin::DELAY:
                        /* fall through */
                    case core::Builtin::WRITE:
                        /* fall through */
                    case core::Builtin::CLEAR:
                        /* fall through */
                    case core::Builtin::WRITE_BOX:
                        return;
                    default:;  // Do nothing
                }
            }
            default:;  // Do nothing
        }

        advance();
    }
}

std::optional<core::Base> Parser::primitiveFromToken(
    Token &token
) {
    switch (token.getType()) {
        case Token::Type::BOOL_TYPE:
            return {core::Base::BOOL};
        case Token::Type::INTEGER_TYPE:
            return {core::Base::INT};
        case Token::Type::FLOAT_TYPE:
            return {core::Base::FLOAT};
        case Token::Type::COLOR_TYPE:
            return {core::Base::COLOR};
        default:
            return {};
    }
}

std::optional<core::Operation> Parser::operationFromToken(
    Token &token
) {
    switch (token.getType()) {
        case Token::Type::PLUS:
            return {core::Operation::ADD};
        case Token::Type::AND:
            return {core::Operation::AND};
        case Token::Type::SLASH:
            return {core::Operation::DIV};
        case Token::Type::EQUAL_EQUAL:
            return {core::Operation::EQ};
        case Token::Type::GREATER_EQUAL:
            return {core::Operation::GE};
        case Token::Type::GREATER:
            return {core::Operation::GT};
        case Token::Type::LESS:
            return {core::Operation::LT};
        case Token::Type::LESS_EQUAL:
            return {core::Operation::LE};
        case Token::Type::STAR:
            return {core::Operation::MUL};
        case Token::Type::BANG_EQUAL:
            return {core::Operation::NEQ};
        case Token::Type::NOT:
            return {core::Operation::NOT};
        case Token::Type::OR:
            return {core::Operation::OR};
        case Token::Type::MINUS:
            return {core::Operation::SUB};
        default:
            return {};
    }
}

}  // namespace PArL
