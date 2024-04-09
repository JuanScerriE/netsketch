#pragma once

// parl
#include <parl/Core.hpp>
#include <parl/Visitor.hpp>

// std
#include <memory>
#include <optional>
#include <vector>

namespace PArL::core {

struct Node {
    virtual void accept(Visitor*) = 0;

    virtual ~Node() = default;

    Position position{0, 0};
};

struct Type : public Node {
    explicit Type(const Base&, bool, std::unique_ptr<IntegerLiteral>);

    void accept(Visitor*) override;

    const Base base;
    const bool isArray;
    std::unique_ptr<IntegerLiteral> size;
};

struct Expr : public Node {
    void accept(Visitor*) override;

    std::optional<std::unique_ptr<Type>> type{};
};

struct Literal : public Expr {};

struct PadWidth : public Literal {
    explicit PadWidth();

    void accept(Visitor*) override;
};

struct PadHeight : public Literal {
    explicit PadHeight();

    void accept(Visitor*) override;
};

struct PadRead : public Literal {
    explicit PadRead(std::unique_ptr<Expr>, std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> x;
    std::unique_ptr<Expr> y;
};

struct PadRandomInt : public Literal {
    explicit PadRandomInt(std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> max;
};

struct BooleanLiteral : public Literal {
    explicit BooleanLiteral(bool);

    void accept(Visitor*) override;

    const bool value;
};

struct IntegerLiteral : public Literal {
    explicit IntegerLiteral(int);

    void accept(Visitor*) override;

    const int value;
};

struct FloatLiteral : public Literal {
    explicit FloatLiteral(float);

    void accept(Visitor*) override;

    const float value;
};

struct ColorLiteral : public Literal {
    explicit ColorLiteral(const Color&);

    void accept(Visitor*) override;

    const Color value;
};

struct ArrayLiteral : public Literal {
    explicit ArrayLiteral(std::vector<
                          std::unique_ptr<Expr>>);

    void accept(Visitor*) override;

    std::vector<std::unique_ptr<Expr>> exprs;
};

struct Reference : public Expr {};

struct Variable : public Reference {
    explicit Variable(std::string);

    void accept(Visitor*) override;

    const std::string identifier;
};

struct ArrayAccess : public Reference {
    explicit ArrayAccess(std::string, std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    const std::string identifier;
    std::unique_ptr<Expr> index;
};

struct FunctionCall : public Reference {
    explicit FunctionCall(std::string, std::vector<std::unique_ptr<Expr>>);

    void accept(Visitor*) override;

    const std::string identifier;
    std::vector<std::unique_ptr<Expr>> params;
};

struct SubExpr : public Expr {
    explicit SubExpr(std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> subExpr;
};

struct Binary : public Expr {
    explicit Binary(std::unique_ptr<Expr>, Operation, std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> left;
    const Operation op;
    std::unique_ptr<Expr> right;
};

struct Unary : public Expr {
    explicit Unary(Operation, std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    const Operation op;
    std::unique_ptr<Expr> expr;
};

struct Stmt : public Node {};

struct Assignment : public Stmt {
    explicit Assignment(std::string, std::unique_ptr<Expr>, std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    const std::string identifier;
    std::unique_ptr<Expr> index;
    std::unique_ptr<Expr> expr;
};

struct VariableDecl : public Stmt {
    explicit VariableDecl(std::string, std::unique_ptr<Type>, std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    const std::string identifier;
    std::unique_ptr<Type> type;
    std::unique_ptr<Expr> expr;
};

struct PrintStmt : public Stmt {
    explicit PrintStmt(std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> expr;
};

struct DelayStmt : public Stmt {
    explicit DelayStmt(std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> expr;
};

struct WriteBoxStmt : public Stmt {
    explicit WriteBoxStmt(std::unique_ptr<Expr>, std::unique_ptr<Expr>, std::unique_ptr<Expr>, std::unique_ptr<Expr>, std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> x;
    std::unique_ptr<Expr> y;
    std::unique_ptr<Expr> w;
    std::unique_ptr<Expr> h;
    std::unique_ptr<Expr> color;
};

struct WriteStmt : public Stmt {
    explicit WriteStmt(std::unique_ptr<Expr>, std::unique_ptr<Expr>, std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> x;
    std::unique_ptr<Expr> y;
    std::unique_ptr<Expr> color;
};

struct ClearStmt : public Stmt {
    explicit ClearStmt(std::unique_ptr<Expr>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> color;
};

struct Block : public Stmt {
    explicit Block(std::vector<std::unique_ptr<Stmt>>);

    void accept(Visitor*) override;

    std::vector<std::unique_ptr<Stmt>> stmts;
};

struct FormalParam : public Node {
    explicit FormalParam(std::string, std::unique_ptr<Type>);

    void accept(Visitor*) override;

    const std::string identifier;
    std::unique_ptr<Type> type;
};

struct FunctionDecl : public Stmt {
    explicit FunctionDecl(std::string, std::vector<std::unique_ptr<FormalParam>>, std::unique_ptr<Type>, std::unique_ptr<Block>);

    void accept(Visitor*) override;

    const std::string identifier;
    std::vector<std::unique_ptr<FormalParam>> params;
    std::unique_ptr<Type> type;
    std::unique_ptr<Block> block;
};

struct IfStmt : public Stmt {
    explicit IfStmt(std::unique_ptr<Expr>, std::unique_ptr<Block>, std::unique_ptr<Block>);

    void accept(Visitor*) override;

    std::unique_ptr<Expr> cond;
    std::unique_ptr<Block> thenBlock;
    std::unique_ptr<Block> elseBlock;
};

struct ForStmt : public Stmt {
    explicit ForStmt(std::unique_ptr<VariableDecl>, std::unique_ptr<Expr>, std::unique_ptr<Assignment>, std::unique_ptr<Block>);

    void accept(Visitor* visitor) override;

    std::unique_ptr<VariableDecl> decl;
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Assignment> assignment;
    std::unique_ptr<Block> block;
};

struct WhileStmt : public Stmt {
    explicit WhileStmt(std::unique_ptr<Expr>, std::unique_ptr<Block>);

    void accept(Visitor* visitor) override;

    std::unique_ptr<Expr> cond;
    std::unique_ptr<Block> block;
};

struct ReturnStmt : public Stmt {
    explicit ReturnStmt(std::unique_ptr<Expr>);

    void accept(Visitor* visitor) override;

    std::unique_ptr<Expr> expr;
};

struct Program : public Node {
    explicit Program(std::vector<std::unique_ptr<Stmt>>);

    void accept(Visitor* visitor) override;

    std::vector<std::unique_ptr<Stmt>> stmts;
};

}  // namespace PArL::core
