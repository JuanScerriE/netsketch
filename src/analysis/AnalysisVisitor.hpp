#pragma once

// parl
#include <analysis/EnvStack.hpp>
#include <backend/Environment.hpp>
#include <backend/Symbol.hpp>
#include <parl/Core.hpp>
#include <parl/Visitor.hpp>
#include <preprocess/IsFunctionVisitor.hpp>

namespace PArL {

class SyncAnalysis : public std::exception {};

class AnalysisVisitor : public core::Visitor {
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

    void analyse(core::Program *);

    void registerFunction(core::FunctionDecl *);

    void isViableCast(
        core::Primitive &from,
        core::Primitive &to
    );

    std::optional<Symbol> findSymbol(
        const std::string &identifier,
        Environment *stoppingEnv
    );

    std::optional<Environment *> findEnclosingEnv(
        Environment::Type envType
    );

    template <typename... T>
    void error(
        const core::Position &position,
        fmt::format_string<T...> fmt,
        T &&...args
    ) {
        mHasError = true;

        fmt::println(
            stderr,
            "semantic error at {}:{}:: {}",
            position.row(),
            position.col(),
            fmt::format(fmt, args...)
        );

        throw SyncAnalysis{};
    }

    [[nodiscard]] bool hasError() const;

    [[nodiscard]] std::unique_ptr<Environment>
    getEnvironment();

   private:
    IsFunctionVisitor isFunction{};
    bool mHasError{false};
    core::Position mPosition{0, 0};
    core::Primitive mReturn{};
    EnvStack mEnvStack{};
};

}  // namespace PArL
