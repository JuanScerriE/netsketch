#pragma once

// fmt
#include <fmt/core.h>

// parl
#include <parl/Visitor.hpp>

namespace PArL {

class PrinterVisitor : public core::Visitor {
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

    template <typename... T>
    inline void print_with_tabs(
        fmt::format_string<T...> fmt,
        T &&...args
    ) {
        for (int i = 0; i < mTabCount; i++) {
            fmt::print("  ");
        }

        fmt::println(fmt, args...);
    }

   private:
    int mTabCount = 0;
};

}  // namespace PArL
