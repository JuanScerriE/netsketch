// fmt
#include <fmt/core.h>
#include <fmt/format.h>

// parl
#include <backend/Environment.hpp>
#include <ir_gen/GenVisitor.hpp>
#include <parl/AST.hpp>
#include <parl/Core.hpp>

namespace PArL {

GenVisitor::GenVisitor(Environment *global) {
    mRefStack.init(global);
}

void GenVisitor::visit(core::Type *) {
    core::abort("unimplemented");
}

void GenVisitor::visit(core::Expr *expr) {
    core::abort("unimplemented");
}

void GenVisitor::visit(core::PadWidth *expr) {
    emit_line("width");
}

void GenVisitor::visit(core::PadHeight *expr) {
    emit_line("height");
}

void GenVisitor::visit(core::PadRead *expr) {
    expr->y->accept(this);
    expr->x->accept(this);
    emit_line("read");
}

void GenVisitor::visit(core::PadRandomInt *expr) {
    expr->max->accept(this);
    emit_line("irnd");
}

void GenVisitor::visit(core::BooleanLiteral *expr) {
    emit_line("push {}", expr->value ? 1 : 0);
}

void GenVisitor::visit(core::IntegerLiteral *expr) {
    emit_line("push {}", expr->value);
}

void GenVisitor::visit(core::FloatLiteral *expr) {
    emit_line("push {}", expr->value);
}

void GenVisitor::visit(core::ColorLiteral *expr) {
    emit_line(
        "push #{:0>2x}{:0>2x}{:0>2x}",
        expr->value.r(),
        expr->value.g(),
        expr->value.b()
    );
}

void GenVisitor::visit(core::ArrayLiteral *expr) {
    for (auto itr = expr->exprs.rbegin();
         itr != expr->exprs.rend();
         itr++) {
        (*itr)->accept(this);
    }
}

void GenVisitor::visit(core::Variable *expr) {
    Environment *env = mRefStack.currentEnv();

    Environment *stoppingEnv = env;

    while (stoppingEnv->getType() !=
               Environment::Type::GLOBAL &&
           stoppingEnv->getType() !=
               Environment::Type::FUNCTION) {
        stoppingEnv = stoppingEnv->getEnclosing();
    }

    std::optional<Symbol> info{};

    for (;;) {
        info = env->findSymbol(expr->identifier);

        if (info.has_value() || env == stoppingEnv)
            break;

        env = env->getEnclosing();
    }

    core::abort_if(
        !info.has_value(),
        "symbol is undefined"
    );

    size_t level = computeLevel(env);

    auto symbol = info->as<VariableSymbol>();

    if (symbol.type.is<core::Base>()) {
        emit_line("push [{}:{}]", symbol.idx, level);

        return;
    }

    if (symbol.type.is<core::Array>()) {
        size_t arraySize =
            symbol.type.as<core::Array>().size;
        emit_line("push {}", arraySize);
        emit_line("pusha [{}:{}]", symbol.idx, level);
        emit_line("push {} // START HACK", arraySize);
        emit_line("oframe");
        emit_line("push {}", arraySize);
        emit_line("push 0");
        emit_line("push 0");
        emit_line("sta");
        emit_line("push {}", arraySize);
        emit_line("pusha [0:0]");
        emit_line("cframe // END HACK");

        return;
    }

    core::abort("unknown type");
}

void GenVisitor::visit(core::ArrayAccess *expr) {
    Environment *env = mRefStack.currentEnv();

    Environment *stoppingEnv = env;

    while (stoppingEnv->getType() !=
               Environment::Type::GLOBAL &&
           stoppingEnv->getType() !=
               Environment::Type::FUNCTION) {
        stoppingEnv = stoppingEnv->getEnclosing();
    }

    std::optional<Symbol> info{};

    for (;;) {
        info = env->findSymbol(expr->identifier);

        if (info.has_value() || env == stoppingEnv)
            break;

        env = env->getEnclosing();
    }

    core::abort_if(
        !info.has_value(),
        "symbol is undefined"
    );

    size_t level = computeLevel(env);

    expr->index->accept(this);

    auto symbol = info->as<VariableSymbol>();

    emit_line("push +[{}:{}]", symbol.idx, level);
}

void GenVisitor::visit(core::FunctionCall *expr) {
    for (auto itr = expr->params.rbegin();
         itr != expr->params.rend();
         itr++) {
        (*itr)->accept(this);
    }

    size_t size{0};

    for (auto &param : expr->params) {
        core::Primitive paramType = mType.getType(
            param.get(),
            mRefStack.getGlobal(),
            mRefStack.currentEnv()
        );

        size += paramType.is<core::Array>()
                    ? paramType.as<core::Array>().size
                    : 1;
    }

    emit_line("push {}", size);
    emit_line("push .{}", expr->identifier);
    emit_line("call");
}

void GenVisitor::visit(core::SubExpr *expr) {
    expr->subExpr->accept(this);
}

void GenVisitor::visit(core::Binary *expr) {
    switch (expr->op) {
        case core::Operation::AND:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("and");
            break;
        case core::Operation::OR:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("or");
            break;
        case core::Operation::LT:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("lt");
            break;
        case core::Operation::GT:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("gt");
            break;
        case core::Operation::EQ:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("eq");
            break;
        case core::Operation::NEQ:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("neq");
            break;
        case core::Operation::LE:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("le");
            break;
        case core::Operation::GE:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("ge");
            break;
        case core::Operation::ADD: {
            core::Primitive type = mType.getType(
                expr->right.get(),
                mRefStack.getGlobal(),
                mRefStack.currentEnv()
            );
            if (type ==
                core::Primitive{core::Base::COLOR}) {
                emit_line("push 16777216");  // #ffffff + 1
                expr->right->accept(this);
                expr->left->accept(this);
                emit_line("add");
                emit_line("mod");
            } else {
                expr->right->accept(this);
                expr->left->accept(this);
                emit_line("add");
            }

        } break;
        case core::Operation::SUB: {
            core::Primitive type = mType.getType(
                expr->right.get(),
                mRefStack.getGlobal(),
                mRefStack.currentEnv()
            );
            if (type ==
                core::Primitive{core::Base::COLOR}) {
                emit_line("push 16777216");
                expr->right->accept(this);
                expr->left->accept(this);
                emit_line("sub");
                emit_line("mod");
            } else {
                expr->right->accept(this);
                expr->left->accept(this);
                emit_line("sub");
            }
        } break;
        case core::Operation::MUL:
            expr->right->accept(this);
            expr->left->accept(this);
            emit_line("mul");
            break;
        case core::Operation::DIV: {
            core::Primitive type = mType.getType(
                expr->right.get(),
                mRefStack.getGlobal(),
                mRefStack.currentEnv()
            );
            if (type == core::Primitive{core::Base::INT}) {
                expr->right->accept(this);
                expr->right->accept(this);
                expr->left->accept(this);
                emit_line("mod");
                expr->left->accept(this);
                emit_line("sub");
                emit_line("div");
            } else {
                expr->right->accept(this);
                expr->left->accept(this);
                emit_line("div");
            }
        } break;
        default:
            core::abort("unreachable");
    }
}

void GenVisitor::visit(core::Unary *expr) {
    expr->expr->accept(this);

    switch (expr->op) {
        case core::Operation::NOT:
            emit_line("not");
            break;
        case core::Operation::SUB: {
            core::Primitive type = mType.getType(
                expr->expr.get(),
                mRefStack.getGlobal(),
                mRefStack.currentEnv()
            );
            if (type ==
                core::Primitive{core::Base::COLOR}) {
                emit_line("push #ffffff");
                emit_line("sub");
            } else {
                emit_line("push -1");
                emit_line("mul");
            }
        } break;
        default:
            core::abort("unreachable");
    }
}

void GenVisitor::visit(core::Assignment *stmt) {
    stmt->expr->accept(this);

    Environment *env = mRefStack.currentEnv();

    Environment *stoppingEnv = env;

    while (stoppingEnv->getType() !=
               Environment::Type::GLOBAL &&
           stoppingEnv->getType() !=
               Environment::Type::FUNCTION) {
        stoppingEnv = stoppingEnv->getEnclosing();
    }

    std::optional<Symbol> left{};

    for (;;) {
        left = env->findSymbol(stmt->identifier);

        if (left.has_value() || env == stoppingEnv)
            break;

        env = env->getEnclosing();
    }

    core::abort_if(
        !left.has_value(),
        "symbol is undefined"
    );

    size_t level = computeLevel(env);

    auto symbol = left->as<VariableSymbol>();

    if (!stmt->index) {
        if (symbol.type.is<core::Array>()) {
            emit_line(
                "push {}",
                symbol.type.as<core::Array>().size
            );
        }
        emit_line(
            "push {}",
            left->as<VariableSymbol>().idx
        );
        emit_line("push {}", level);
        if (symbol.type.is<core::Array>()) {
            emit_line("sta");
        } else {
            emit_line("st");
        }
    } else {
        stmt->index->accept(this);

        emit_line(
            "push {}",
            left->as<VariableSymbol>().idx
        );
        emit_line("add");
        emit_line("push {}", level);
        emit_line("st");
    }
}

void GenVisitor::visit(core::VariableDecl *stmt) {
    stmt->expr->accept(this);

    Environment *env = mRefStack.currentEnv();

    Environment *stoppingEnv = env;

    while (stoppingEnv->getType() !=
               Environment::Type::GLOBAL &&
           stoppingEnv->getType() !=
               Environment::Type::FUNCTION) {
        stoppingEnv = stoppingEnv->getEnclosing();
    }

    std::optional<Symbol> left{};

    for (;;) {
        left = env->findSymbol(stmt->identifier);

        if (left.has_value() || env == stoppingEnv)
            break;

        env = env->getEnclosing();
    }

    core::abort_if(
        !left.has_value(),
        "symbol is undefined"
    );

    size_t idx = env->getIdx();

    auto symbol = left->as<VariableSymbol>();

    if (symbol.type.is<core::Base>()) {
        env->incIdx();
    } else if (symbol.type.is<core::Array>()) {
        env->incIdx(symbol.type.as<core::Array>().size);
    } else {
        core::abort("unknown type");
    }

    // NOTE: make sure this is actually a reference.
    env->getSymbolAsRef(stmt->identifier)
        .asRef<VariableSymbol>()
        .idx = idx;

    if (symbol.type.is<core::Array>()) {
        emit_line(
            "push {}",
            symbol.type.as<core::Array>().size
        );
    }
    emit_line("push {}", idx);
    emit_line("push 0");
    if (symbol.type.is<core::Array>()) {
        emit_line("sta");
    } else {
        emit_line("st");
    }
}

void GenVisitor::visit(core::PrintStmt *stmt) {
    stmt->expr->accept(this);

    core::Primitive type = mType.getType(
        stmt->expr.get(),
        mRefStack.getGlobal(),
        mRefStack.currentEnv()
    );

    if (type.is<core::Base>()) {
        emit_line("print");

        return;
    }

    if (type.is<core::Array>()) {
        emit_line("push {}", type.as<core::Array>().size);
        emit_line("printa");

        return;
    }

    core::abort("unknown type");
}

void GenVisitor::visit(core::DelayStmt *stmt) {
    stmt->expr->accept(this);

    emit_line("delay");
}

void GenVisitor::visit(core::WriteBoxStmt *stmt) {
    stmt->color->accept(this);
    stmt->h->accept(this);
    stmt->w->accept(this);
    stmt->y->accept(this);
    stmt->x->accept(this);

    emit_line("writebox");
}

void GenVisitor::visit(core::WriteStmt *stmt) {
    stmt->color->accept(this);
    stmt->y->accept(this);
    stmt->x->accept(this);

    emit_line("write");
}

void GenVisitor::visit(core::ClearStmt *stmt) {
    stmt->color->accept(this);

    emit_line("clear");
}

void GenVisitor::visit(core::Block *block) {
    Environment *nextEnv = mRefStack.peekNextEnv();

    size_t count = 0;

    for (auto &stmt : block->stmts) {
        count += mDeclCounter.count(stmt.get(), nextEnv);
    }

    emit_line("push {}", count);
    emit_line("oframe");

    mFrameDepth++;

    mRefStack.pushEnv(count);

    for (auto &stmt : block->stmts) {
        stmt->accept(this);
    }

    mRefStack.popEnv();

    mFrameDepth--;

    emit_line("cframe");
}

void GenVisitor::visit(core::FormalParam *param) {
    Environment *env = mRefStack.currentEnv();

    std::optional<Symbol> symbol =
        env->findSymbol(param->identifier);

    core::abort_if(
        !symbol.has_value(),
        "symbol is undefined"
    );

    size_t idx = env->getIdx();

    auto variable = symbol->as<VariableSymbol>();

    if (variable.type.is<core::Base>()) {
        env->incIdx();
    } else if (variable.type.is<core::Array>()) {
        env->incIdx(variable.type.as<core::Array>().size);
    } else {
        core::abort("unknown type");
    }

    // NOTE: make sure this is actually a reference.
    env->getSymbolAsRef(param->identifier)
        .asRef<VariableSymbol>()
        .idx = idx;
}

void GenVisitor::visit(core::FunctionDecl *stmt) {
    Environment *nextEnv = mRefStack.peekNextEnv();

    size_t aritySize{0};

    for (auto &stmt : stmt->params) {
        aritySize +=
            mDeclCounter.count(stmt.get(), nextEnv);
    }

    emit_line(".{}", stmt->identifier);

    mRefStack.pushEnv(aritySize);

    for (auto &param : stmt->params) {
        param->accept(this);
    }

    stmt->block->accept(this);

    mRefStack.popEnv();
}

void GenVisitor::visit(core::IfStmt *stmt) {
    if (stmt->elseBlock) {
        mRefStack.pushEnv();

        stmt->cond->accept(this);

        emit_line("not");

        size_t ifPatchOffset = PC();

        emit_line("push #PC+{{}}");
        emit_line("cjmp");

        stmt->thenBlock->accept(this);

        mRefStack.popEnv();

        mRefStack.pushEnv();

        size_t elsePatchOffset = PC();

        emit_line("push #PC+{{}}");
        emit_line("jmp");

        mCode[ifPatchOffset] = fmt::format(
            mCode[ifPatchOffset],
            PC() - ifPatchOffset
        );

        stmt->elseBlock->accept(this);

        mCode[elsePatchOffset] = fmt::format(
            mCode[elsePatchOffset],
            PC() - elsePatchOffset
        );

        mRefStack.popEnv();
    } else {
        mRefStack.pushEnv();

        stmt->cond->accept(this);

        emit_line("not");

        size_t patchOffset = PC();

        emit_line("push #PC+{{}}");
        emit_line("cjmp");

        stmt->thenBlock->accept(this);

        mCode[patchOffset] = fmt::format(
            mCode[patchOffset],
            PC() - patchOffset
        );

        mRefStack.popEnv();
    }
}

void GenVisitor::visit(core::ForStmt *stmt) {
    size_t count = stmt->decl ? mDeclCounter.count(
                                    stmt->decl.get(),
                                    mRefStack.peekNextEnv()
                                )
                              : 0;

    emit_line("push {}", count);
    emit_line("oframe");

    mFrameDepth++;

    mRefStack.pushEnv(count);

    if (stmt->decl) {
        stmt->decl->accept(this);
    }

    size_t condOffset = PC();

    stmt->cond->accept(this);

    emit_line("not");

    size_t patchOffset = PC();

    emit_line("push #PC+{{}}");
    emit_line("cjmp");

    stmt->block->accept(this);

    stmt->assignment->accept(this);

    emit_line("push #PC-{}", PC() - condOffset);
    emit_line("jmp");

    mCode[patchOffset] =
        fmt::format(mCode[patchOffset], PC() - patchOffset);

    mRefStack.popEnv();

    mFrameDepth--;

    emit_line("cframe");
}

void GenVisitor::visit(core::WhileStmt *stmt) {
    mRefStack.pushEnv();

    size_t condOffset = PC();

    stmt->cond->accept(this);

    emit_line("not");

    size_t patchOffset = PC();

    emit_line("push #PC+{{}}");
    emit_line("cjmp");

    stmt->block->accept(this);

    emit_line("push #PC-{}", PC() - condOffset);
    emit_line("jmp");

    mCode[patchOffset] =
        fmt::format(mCode[patchOffset], PC() - patchOffset);

    mRefStack.popEnv();
}

void GenVisitor::visit(core::ReturnStmt *stmt) {
    stmt->expr->accept(this);

    for (size_t i = 0; i < mFrameDepth; i++) {
        emit_line("cframe");
    }

    emit_line("ret");
}

void GenVisitor::visit(core::Program *prog) {
    auto itr = prog->stmts.begin();

    for (; itr != prog->stmts.end(); itr++) {
        if (isFunction.check(itr->get())) {
            (*itr)->accept(this);
        } else {
            break;
        }
    }

    size_t count = 0;

    auto itr_ = itr;

    for (; itr_ != prog->stmts.end(); itr_++) {
        count += mDeclCounter.count(
            itr_->get(),
            mRefStack.currentEnv()
        );
    }

    emit_line(".main");
    emit_line("push {}", count);
    emit_line("oframe");

    mFrameDepth++;

    mRefStack.currentEnv()->setSize(count);

    for (; itr != prog->stmts.end(); itr++) {
        core::abort_if(
            isFunction.check(itr->get()),
            "no function declaration allowed in .main"
        );

        (*itr)->accept(this);
    }

    mFrameDepth--;

    emit_line("cframe");
    emit_line("halt");
}

// the below is an example of program which does not
// properly close its scopes .test push 1 oframe push 44
// push 0
// push 0
// st
// push 1
// oframe
// push 20
// push 0
// push 0
// st
// push 10
// ret
// cframe
// push 20
// cframe
// ret
// .main
// push 0
// push .test
// call
// print
// push [0:0]
// print
// halt

void GenVisitor::print() {
    for (auto &line : mCode) {
        fmt::println(line);
    }
}

size_t GenVisitor::computeLevel(Environment *stoppingEnv) {
    size_t level = 0;

    Environment *env = mRefStack.currentEnv();

    while (stoppingEnv != env) {
        switch (env->getType()) {
            case Environment::Type::FOR:
            case Environment::Type::BLOCK:
                level++;
                break;
            case Environment::Type::FUNCTION:
                core::abort("unreachable");
                break;
            default:
                // noop
                break;
        }

        env = env->getEnclosing();
    }

    return level;
}

size_t GenVisitor::PC() const {
    return mCode.size();
}

void GenVisitor::reset() {
    isFunction.reset();
    mDeclCounter.reset();
    mRefStack.reset();
    mCode.clear();
    mFrameDepth = 0;
}

}  // namespace PArL
