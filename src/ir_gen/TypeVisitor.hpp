#pragma once

// parl
#include <ir_gen/RefStack.hpp>
#include <parl/AST.hpp>
#include <parl/Core.hpp>
#include <parl/Visitor.hpp>

namespace PArL {

class TypeVisitor : public core::Visitor {
   public:
    // expression types
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
    // statement types
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

    core::Primitive getType(
        core::Node *node,
        Environment *global,
        Environment *current
    );

    std::optional<Symbol> findSymbol(
        const std::string &identifier,
        Environment *stoppingEnv
    );

    std::optional<Environment *> findEnclosingEnv(
        Environment::Type envType
    );

   private:
    core::Primitive mReturn{};
    RefStack mRefStack;
};

}  // namespace PArL
