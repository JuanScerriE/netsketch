// parl
#include <analysis/ReturnVisitor.hpp>
#include <parl/AST.hpp>

#include "parl/Core.hpp"

namespace PArL {

void ReturnVisitor::visit(core::Type *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::Expr *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::PadWidth *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::PadHeight *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::PadRead *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::PadRandomInt *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::BooleanLiteral *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::IntegerLiteral *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::FloatLiteral *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::ColorLiteral *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::ArrayLiteral *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::Variable *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::ArrayAccess *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::FunctionCall *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::SubExpr *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::Binary *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::Unary *) {
    core::abort("unimplemented");
}

void ReturnVisitor::visit(core::Assignment *) {
    // noop
}

void ReturnVisitor::visit(core::VariableDecl *stmt) {
    // noop
}

void ReturnVisitor::visit(core::PrintStmt *) {
    // noop
}

void ReturnVisitor::visit(core::DelayStmt *) {
    // noop
}

void ReturnVisitor::visit(core::WriteBoxStmt *) {
    // noop
}

void ReturnVisitor::visit(core::WriteStmt *) {
    // noop
}

void ReturnVisitor::visit(core::ClearStmt *) {
    // noop
}

void ReturnVisitor::visit(core::Block *block) {
    size_t i = 0;

    for (; i < block->stmts.size(); i++) {
        block->stmts[i]->accept(this);

        if (mBranchReturns) {
            i++;

            break;
        }
    }

    if (i < block->stmts.size()) {
        core::Position start = block->stmts[i]->position;

        warning(
            start,
            "statements starting from line {} upto line {} "
            "are unreachable",
            start.row(),
            block->position.row()
        );
    }

    for (; i < block->stmts.size(); i++) {
        block->stmts.pop_back();
    }
}

void ReturnVisitor::visit(core::FormalParam *param) {
    // noop
}

void ReturnVisitor::visit(core::FunctionDecl *stmt) {
    mBranchReturns = false;

    stmt->block->accept(this);

    if (!mBranchReturns) {
        error(
            stmt->position,
            "{}(...) does not return a value in all "
            "control paths",
            stmt->identifier
        );
    }

    mBranchReturns = false;
}

void ReturnVisitor::visit(core::IfStmt *stmt) {
    stmt->thenBlock->accept(this);
    bool thenBranch = mBranchReturns;

    bool elseBranch = false;

    if (stmt->elseBlock) {
        stmt->elseBlock->accept(this);
        elseBranch = mBranchReturns;
    }

    mBranchReturns = thenBranch && elseBranch;
}

void ReturnVisitor::visit(core::ForStmt *stmt) {
    stmt->block->accept(this);

    mBranchReturns = false;
}

void ReturnVisitor::visit(core::WhileStmt *stmt) {
    stmt->block->accept(this);

    mBranchReturns = false;
}

void ReturnVisitor::visit(core::ReturnStmt *stmt) {
    mBranchReturns = true;
}

void ReturnVisitor::visit(core::Program *prog) {
    for (auto &stmt : prog->stmts) {
        stmt->accept(this);
    }
}

void ReturnVisitor::reset() {
    mHasError = false;
    mBranchReturns = false;
}

bool ReturnVisitor::hasError() const {
    return mHasError;
}

}  // namespace PArL
