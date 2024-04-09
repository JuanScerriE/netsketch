#pragma once

// std
#include <deque>
#include <memory>

// parl
#include <backend/Environment.hpp>
#include <parl/AST.hpp>
#include <parl/Visitor.hpp>
#include <preprocess/IsFunctionVisitor.hpp>

namespace PArL {

class ReorderVisitor : public core::Visitor {
   public:
    void visit(core::Type *) override;
    void visit(core::Expr *) override;
    void visit(core::PadWidth *) override;
    void visit(core::PadHeight *) override;
    void visit(core::PadRead *) override;
    void visit(core::PadRandomInt *) override;
    void visit(core::BooleanLiteral *) override;
    void visit(core::IntegerLiteral *) override;
    void visit(core::FloatLiteral *) override;
    void visit(core::ColorLiteral *) override;
    void visit(core::ArrayLiteral *) override;
    void visit(core::Variable *) override;
    void visit(core::ArrayAccess *) override;
    void visit(core::FunctionCall *) override;
    void visit(core::SubExpr *) override;
    void visit(core::Binary *) override;
    void visit(core::Unary *) override;
    void visit(core::Assignment *) override;
    void visit(core::VariableDecl *) override;
    void visit(core::PrintStmt *) override;
    void visit(core::DelayStmt *) override;
    void visit(core::WriteBoxStmt *) override;
    void visit(core::WriteStmt *) override;
    void visit(core::ClearStmt *) override;
    void visit(core::Block *) override;
    void visit(core::FormalParam *) override;
    void visit(core::FunctionDecl *) override;
    void visit(core::IfStmt *) override;
    void visit(core::ForStmt *) override;
    void visit(core::WhileStmt *) override;
    void visit(core::ReturnStmt *) override;
    void visit(core::Program *) override;

    void reset() override;

    void reorder(
        std::vector<std::unique_ptr<core::Stmt>> &stmts
    );

    void reorderAst(core::Program *);

    void reorderEnvironment(Environment *);

   private:
    IsFunctionVisitor isFunction{};
    std::deque<std::unique_ptr<core::Stmt>> mFuncQueue{};
    std::deque<std::unique_ptr<core::Stmt>> mStmtQueue{};
    std::deque<std::unique_ptr<Environment>> mFuncEnvQueue{
    };
    std::deque<std::unique_ptr<Environment>> mOtherEnvQueue{
    };
};

}  // namespace PArL
