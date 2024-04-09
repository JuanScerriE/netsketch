#pragma once

namespace PArL::core {

struct Type;
struct Expr;
struct PadWidth;
struct PadHeight;
struct PadRead;
struct PadRandomInt;
struct BooleanLiteral;
struct IntegerLiteral;
struct FloatLiteral;
struct ColorLiteral;
struct ArrayLiteral;
struct Variable;
struct ArrayAccess;
struct FunctionCall;
struct SubExpr;
struct Binary;
struct Unary;
struct Assignment;
struct VariableDecl;
struct PrintStmt;
struct DelayStmt;
struct WriteBoxStmt;
struct WriteStmt;
struct ClearStmt;
struct Block;
struct FormalParam;
struct FunctionDecl;
struct IfStmt;
struct ForStmt;
struct WhileStmt;
struct ReturnStmt;
struct Program;

class Visitor {
   public:
    virtual void visit(Type*) = 0;
    virtual void visit(Expr*) = 0;
    virtual void visit(PadWidth*) = 0;
    virtual void visit(PadHeight*) = 0;
    virtual void visit(PadRead*) = 0;
    virtual void visit(PadRandomInt*) = 0;
    virtual void visit(BooleanLiteral*) = 0;
    virtual void visit(IntegerLiteral*) = 0;
    virtual void visit(FloatLiteral*) = 0;
    virtual void visit(ColorLiteral*) = 0;
    virtual void visit(ArrayLiteral*) = 0;
    virtual void visit(Variable*) = 0;
    virtual void visit(ArrayAccess*) = 0;
    virtual void visit(FunctionCall*) = 0;
    virtual void visit(SubExpr*) = 0;
    virtual void visit(Binary*) = 0;
    virtual void visit(Unary*) = 0;
    virtual void visit(Assignment*) = 0;
    virtual void visit(VariableDecl*) = 0;
    virtual void visit(PrintStmt*) = 0;
    virtual void visit(DelayStmt*) = 0;
    virtual void visit(WriteBoxStmt*) = 0;
    virtual void visit(WriteStmt*) = 0;
    virtual void visit(ClearStmt*) = 0;
    virtual void visit(Block*) = 0;
    virtual void visit(FormalParam*) = 0;
    virtual void visit(FunctionDecl*) = 0;
    virtual void visit(IfStmt*) = 0;
    virtual void visit(ForStmt*) = 0;
    virtual void visit(WhileStmt*) = 0;
    virtual void visit(ReturnStmt*) = 0;
    virtual void visit(Program*) = 0;
    virtual void reset() = 0;

    virtual ~Visitor() = default;
};

}  // namespace PArL::core
