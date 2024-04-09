// fmt
#include <fmt/core.h>
#include <fmt/format.h>

// parl
#include <parl/AST.hpp>
#include <preprocess/ReorderVisitor.hpp>

// std
#include <memory>

namespace PArL {

void ReorderVisitor::visit(core::Type *) {
}

void ReorderVisitor::visit(core::Expr *) {
}

void ReorderVisitor::visit(core::PadWidth *) {
}

void ReorderVisitor::visit(core::PadHeight *) {
}

void ReorderVisitor::visit(core::PadRead *) {
}

void ReorderVisitor::visit(core::PadRandomInt *) {
}

void ReorderVisitor::visit(core::BooleanLiteral *) {
}

void ReorderVisitor::visit(core::IntegerLiteral *) {
}

void ReorderVisitor::visit(core::FloatLiteral *) {
}

void ReorderVisitor::visit(core::ColorLiteral *) {
}

void ReorderVisitor::visit(core::ArrayLiteral *) {
}

void ReorderVisitor::visit(core::Variable *) {
}

void ReorderVisitor::visit(core::ArrayAccess *) {
}

void ReorderVisitor::visit(core::FunctionCall *) {
}

void ReorderVisitor::visit(core::SubExpr *) {
}

void ReorderVisitor::visit(core::Binary *) {
}

void ReorderVisitor::visit(core::Unary *) {
}

void ReorderVisitor::visit(core::Assignment *) {
}

void ReorderVisitor::visit(core::VariableDecl *) {
}

void ReorderVisitor::visit(core::PrintStmt *) {
}

void ReorderVisitor::visit(core::DelayStmt *) {
}

void ReorderVisitor::visit(core::WriteBoxStmt *) {
}

void ReorderVisitor::visit(core::WriteStmt *) {
}

void ReorderVisitor::visit(core::ClearStmt *) {
}

void ReorderVisitor::visit(core::Block *block) {
    reorder(block->stmts);

    for (auto &stmt : block->stmts) {
        stmt->accept(this);
    }
}

void ReorderVisitor::visit(core::FormalParam *) {
}

void ReorderVisitor::visit(core::FunctionDecl *stmt) {
    stmt->block->accept(this);
}

void ReorderVisitor::visit(core::IfStmt *stmt) {
    stmt->thenBlock->accept(this);

    if (stmt->elseBlock) {
        stmt->elseBlock->accept(this);
    }
}

void ReorderVisitor::visit(core::ForStmt *stmt) {
    stmt->block->accept(this);
}

void ReorderVisitor::visit(core::WhileStmt *stmt) {
    stmt->block->accept(this);
}

void ReorderVisitor::visit(core::ReturnStmt *) {
}

void ReorderVisitor::visit(core::Program *prog) {
    reorder(prog->stmts);

    for (auto &stmt : prog->stmts) {
        stmt->accept(this);
    }
}

void ReorderVisitor::reorder(
    std::vector<std::unique_ptr<core::Stmt>> &stmts
) {
    for (auto &stmt : stmts) {
        if (isFunction.check(stmt.get())) {
            mFuncQueue.push_back(std::move(stmt));
        } else {
            mStmtQueue.push_back(std::move(stmt));
        }
    }

    stmts.clear();

    for (auto &stmt : mFuncQueue) {
        stmts.push_back(std::move(stmt));
    }

    for (auto &stmt : mStmtQueue) {
        stmts.push_back(std::move(stmt));
    }

    reset();
}

void ReorderVisitor::reorderAst(core::Program *ast) {
    ast->accept(this);
}

void ReorderVisitor::reorderEnvironment(Environment *env) {
    auto &children = env->children();

    for (auto &childEnv : children) {
        if (childEnv->getType() ==
            Environment::Type::FUNCTION) {
            mFuncEnvQueue.push_back(std::move(childEnv));
        } else {
            mOtherEnvQueue.push_back(std::move(childEnv));
        }
    }

    children.clear();

    for (auto &env : mFuncEnvQueue) {
        children.push_back(std::move(env));
    }

    for (auto &env : mOtherEnvQueue) {
        children.push_back(std::move(env));
    }

    reset();

    for (auto &childEnv : children) {
        reorderEnvironment(childEnv.get());
    }
}

void ReorderVisitor::reset() {
    mFuncQueue.clear();
    mStmtQueue.clear();
    mFuncEnvQueue.clear();
    mOtherEnvQueue.clear();
}

}  // namespace PArL
