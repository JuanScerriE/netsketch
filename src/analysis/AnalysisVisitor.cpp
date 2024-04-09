// fmt
#include <fmt/core.h>
#include <fmt/format.h>

// parl
#include <analysis/AnalysisVisitor.hpp>
#include <analysis/ReturnVisitor.hpp>
#include <backend/Environment.hpp>
#include <parl/AST.hpp>
#include <parl/Core.hpp>
#include <parl/Token.hpp>

// std
#include <memory>

namespace PArL {

void AnalysisVisitor::visit(core::Type *type) {
    // TODO: add support for more complicated parsing
    // of types (later on maybe during the summer)
    if (!type->isArray) {
        mReturn = type->base;

        return;
    }

    if (!type->size) {
        error(
            type->position,
            "array size must be specified"
        );
    }

    int value = type->size->value;

    if (value < 1) {
        error(
            type->position,
            "array size must be positive"
        );
    }

    mReturn = core::Array{
        static_cast<size_t>(value),
        core::box{core::Primitive{type->base}}
    };
}

void AnalysisVisitor::visit(core::Expr *expr) {
    if (expr->type.has_value()) {
        core::Type *type = (*expr->type).get();

        type->accept(this);

        mPosition = type->position;
    }
}

void AnalysisVisitor::visit(core::PadWidth *expr) {
    mReturn = core::Base::INT;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::PadHeight *expr) {
    mReturn = core::Base::INT;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::PadRead *expr) {
    expr->x->accept(this);
    auto xType{mReturn};

    expr->y->accept(this);
    auto yType{mReturn};

    if (xType != core::Primitive{core::Base::INT}) {
        error(
            expr->position,
            "__read expects x to be an integer"
        );
    }

    if (yType != core::Primitive{core::Base::INT}) {
        error(
            expr->position,
            "__read expects y to be an integer"
        );
    }

    mReturn = core::Base::COLOR;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::PadRandomInt *expr) {
    expr->max->accept(this);
    auto maxType{mReturn};

    if (maxType != core::Primitive{core::Base::INT}) {
        error(
            expr->position,
            "__random_int expects max to be an integer"
        );
    }

    mReturn = core::Base::INT;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::BooleanLiteral *expr) {
    mReturn = core::Base::BOOL;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::IntegerLiteral *expr) {
    mReturn = core::Base::INT;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::FloatLiteral *expr) {
    mReturn = core::Base::FLOAT;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::ColorLiteral *expr) {
    mReturn = core::Base::COLOR;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::ArrayLiteral *expr) {
    if (expr->exprs.empty()) {
        error(expr->position, "array literal is empty");
    }

    expr->exprs[0]->accept(this);
    auto initialType{mReturn};

    // NOTE: this is a restriction which we impose on
    // ourselves to increase simplicity later on
    if (initialType.is<core::Array>()) {
        error(
            expr->position,
            "nested arrays are not supported"
        );
    }

    for (size_t i = 1; i < expr->exprs.size(); i++) {
        expr->exprs[i]->accept(this);

        if (mReturn.is<core::Array>()) {
            error(
                expr->position,
                "nested arrays are not supported"
            );
        }

        if (initialType != mReturn) {
            error(
                expr->position,
                "array contains multiple different types"
            );
        }
    }

    mReturn = core::Array{
        expr->exprs.size(),
        core::box{initialType},
    };
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::Variable *expr) {
    std::optional<Environment *> stoppingEnv =
        findEnclosingEnv(Environment::Type::FUNCTION);

    std::optional<Symbol> symbol{findSymbol(
        expr->identifier,
        stoppingEnv.has_value() ? *stoppingEnv
                                : mEnvStack.getGlobal()
    )};

    if (!symbol.has_value()) {
        error(
            expr->position,
            "{} is undefined",
            expr->identifier
        );
    }

    if (symbol->is<FunctionSymbol>()) {
        error(
            expr->position,
            "{}(...) being used as a variable",
            expr->identifier
        );
    }

    mReturn = symbol->as<VariableSymbol>().type;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::ArrayAccess *expr) {
    std::optional<Environment *> stoppingEnv =
        findEnclosingEnv(Environment::Type::FUNCTION);

    std::optional<Symbol> symbol{findSymbol(
        expr->identifier,
        stoppingEnv.has_value() ? *stoppingEnv
                                : mEnvStack.getGlobal()
    )};

    if (!symbol.has_value()) {
        error(
            expr->position,
            "{} is undefined",
            expr->identifier
        );
    }

    if (symbol->is<FunctionSymbol>()) {
        error(
            expr->position,
            "{}(...) being used as an array",
            expr->identifier
        );
    }

    auto type{symbol->as<VariableSymbol>().type};

    if (!type.is<core::Array>()) {
        error(
            expr->position,
            "{} being used as an array",
            expr->identifier
        );
    }

    expr->index->accept(this);
    auto indexType{mReturn};

    if (indexType != core::Primitive{core::Base::INT}) {
        error(
            expr->position,
            "array {} indexed with non-integer",
            expr->identifier
        );
    }

    mReturn = *type.as<core::Array>().type;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::FunctionCall *expr) {
    std::optional<Symbol> symbol{
        findSymbol(expr->identifier, mEnvStack.getGlobal())
    };

    if (!symbol.has_value()) {
        error(
            expr->position,
            "{}(...) is undefined",
            expr->identifier
        );
    }

    if (!symbol->is<FunctionSymbol>()) {
        error(
            expr->position,
            "{} being used as a function",
            expr->identifier
        );
    }

    auto funcSymbol = symbol->as<FunctionSymbol>();

    std::vector<core::Primitive> paramTypes{};

    for (auto &param : expr->params) {
        param->accept(this);

        paramTypes.push_back(mReturn);
    }

    if (funcSymbol.paramTypes.size() != paramTypes.size()) {
        error(
            expr->position,
            "function {}(...) received {} parameters, "
            "expected {}",
            expr->identifier,
            expr->params.size(),
            funcSymbol.paramTypes.size()
        );
    }

    for (int i = 0; i < paramTypes.size(); i++) {
        if (paramTypes[i] != funcSymbol.paramTypes[i]) {
            error(
                expr->position,
                "function {}(...) received parameter "
                "of unexpected type {}",
                expr->identifier,
                core::primitiveToString(&paramTypes[i])
            );
        }
    }

    mReturn = funcSymbol.returnType;
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::SubExpr *expr) {
    expr->subExpr->accept(this);
    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::Binary *expr) {
    expr->left->accept(this);
    auto leftType{mReturn};

    expr->right->accept(this);
    auto rightType{mReturn};

    switch (expr->op) {
        case core::Operation::AND:
        case core::Operation::OR:
            if (!(leftType ==
                      core::Primitive{core::Base::BOOL} &&
                  rightType ==
                      core::Primitive{core::Base::BOOL})) {
                error(
                    expr->position,
                    "operator {} expects boolean operands",
                    core::operationToString(expr->op)
                );
            }

            mReturn = core::Base::BOOL;
            break;
        case core::Operation::EQ:
        case core::Operation::GE:
        case core::Operation::GT:
        case core::Operation::LE:
        case core::Operation::LT:
        case core::Operation::NEQ:
            if (leftType.is<core::Array>()) {
                error(
                    expr->position,
                    "operator {} is not defined on array "
                    "types",
                    core::operationToString(expr->op)
                );
            }

            if (leftType != rightType) {
                error(
                    expr->position,
                    "operator {} expects both operands to "
                    "be of same type",
                    core::operationToString(expr->op)
                );
            }

            mReturn = core::Base::BOOL;
            break;
        case core::Operation::ADD:
        case core::Operation::SUB:
            if (leftType.is<core::Array>()) {
                error(
                    expr->position,
                    "operator {} is not defined on array "
                    "types",
                    core::operationToString(expr->op)
                );
            }

            if (leftType != rightType) {
                error(
                    expr->position,
                    "operator {} expects both operands to "
                    "be of same type",
                    core::operationToString(expr->op)
                );
            }

            mReturn = leftType;
            break;
        case core::Operation::DIV:
        case core::Operation::MUL:
            if (leftType.is<core::Array>() ||
                rightType.is<core::Array>()) {
                error(
                    expr->position,
                    "operator {} is not defined on array "
                    "types",
                    core::operationToString(expr->op)
                );
            }

            if (leftType ==
                    core::Primitive{core::Base::COLOR} ||
                rightType ==
                    core::Primitive{core::Base::COLOR}) {
                error(
                    expr->position,
                    "operator {} is not defined on color "
                    "type",
                    core::operationToString(expr->op)
                );
            }

            if (leftType != rightType) {
                error(
                    expr->position,
                    "operator {} expects both operands to "
                    "be of same type",
                    core::operationToString(expr->op)
                );
            }

            mReturn = leftType;
            break;
        default:
            core::abort("unreachable");
    }

    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::Unary *expr) {
    expr->expr->accept(this);

    switch (expr->op) {
        case core::Operation::NOT:
            if (mReturn !=
                core::Primitive{core::Base::BOOL}) {
                error(
                    expr->position,
                    "operator {} expects boolean operand",
                    core::operationToString(expr->op)
                );
            }
            break;
        case core::Operation::SUB:
            if (mReturn.is<core::Array>()) {
                error(
                    expr->position,
                    "operator {} is not defined on array "
                    "types",
                    core::operationToString(expr->op)
                );
            }
            if (mReturn ==
                core::Primitive{core::Base::BOOL}) {
                error(
                    expr->position,
                    "operator {} does not expect "
                    "boolean operand",
                    core::operationToString(expr->op)
                );
            }
            break;
        default:
            core::abort("unreachable");
    }

    auto from{mReturn};

    expr->core::Expr::accept(this);
    auto to{mReturn};

    isViableCast(from, to);
}

void AnalysisVisitor::visit(core::PrintStmt *stmt) {
    stmt->expr->accept(this);
}

void AnalysisVisitor::visit(core::DelayStmt *stmt) {
    stmt->expr->accept(this);
    auto delayType{mReturn};

    if (delayType != core::Primitive{core::Base::INT}) {
        error(
            stmt->position,
            "__delay expects delay to be an integer"
        );
    }
}

void AnalysisVisitor::visit(core::WriteBoxStmt *stmt) {
    stmt->x->accept(this);
    auto xType{mReturn};

    stmt->y->accept(this);
    auto yType{mReturn};

    stmt->w->accept(this);
    auto wType{mReturn};

    stmt->h->accept(this);
    auto hType{mReturn};

    stmt->color->accept(this);
    auto colorType{mReturn};

    if (xType != core::Primitive{core::Base::INT}) {
        error(
            stmt->position,
            "__write_box expects x to be an integer"
        );
    }

    if (yType != core::Primitive{core::Base::INT}) {
        error(
            stmt->position,
            "__write_box expects y to be an integer"
        );
    }

    if (wType != core::Primitive{core::Base::INT}) {
        error(
            stmt->position,
            "__write_box expects xOffset to be an integer"
        );
    }

    if (hType != core::Primitive{core::Base::INT}) {
        error(
            stmt->position,
            "__write_box expects yOffset to be an integer"
        );
    }

    if (colorType != core::Primitive{core::Base::COLOR}) {
        error(
            stmt->position,
            "__write_box expects a color"
        );
    }
}

void AnalysisVisitor::visit(core::WriteStmt *stmt) {
    stmt->x->accept(this);
    auto xType{mReturn};

    stmt->y->accept(this);
    auto yType{mReturn};

    stmt->color->accept(this);
    auto colorType{mReturn};

    if (xType != core::Primitive{core::Base::INT}) {
        error(
            stmt->position,
            "__write expects x to be an integer"
        );
    }

    if (yType != core::Primitive{core::Base::INT}) {
        error(
            stmt->position,
            "__write expects y to be an integer"
        );
    }

    if (colorType != core::Primitive{core::Base::COLOR}) {
        error(
            stmt->position,
            "__write expects color to be a color"
        );
    }
}

void AnalysisVisitor::visit(core::ClearStmt *stmt) {
    stmt->color->accept(this);
    auto colorType{mReturn};

    if (colorType != core::Primitive{core::Base::COLOR}) {
        error(
            stmt->position,
            "__clear expects color to be a color"
        );
    }
}

void AnalysisVisitor::visit(core::Assignment *stmt) {
    std::optional<Environment *> stoppingEnv =
        findEnclosingEnv(Environment::Type::FUNCTION);

    std::optional<Symbol> leftSymbol{findSymbol(
        stmt->identifier,
        stoppingEnv.has_value() ? *stoppingEnv
                                : mEnvStack.getGlobal()
    )};

    if (!leftSymbol.has_value()) {
        error(
            stmt->position,
            "{} is undefined",
            stmt->identifier
        );
    }

    if (leftSymbol->is<FunctionSymbol>()) {
        error(
            stmt->position,
            "{}(...) is being in assignment",
            stmt->identifier
        );
    }

    if (stmt->index) {
        stmt->index->accept(this);
        auto indexType{mReturn};

        if (!indexType.is<core::Base>() &&
            indexType != core::Primitive{core::Base::INT}) {
            error(
                stmt->position,
                "{} indexed with non-integer",
                stmt->identifier
            );
        }
    }

    auto leftType = leftSymbol->as<VariableSymbol>().type;

    if (stmt->index && !leftType.is<core::Array>()) {
        error(
            stmt->position,
            "{} is not an array",
            stmt->identifier
        );
    }

    if (stmt->index) {
        core::box<core::Primitive> copy =
            leftType.as<core::Array>().type;
        //        leftPrimitive =
        //            *leftPrimitive.as<core::Array>().type;
        // HACK: make sure that we have proper copying of
        // the above nature
        leftType = *copy;
    }

    stmt->expr->accept(this);
    auto rightType{mReturn};

    if (leftType != rightType) {
        error(
            stmt->position,
            "left-hand side of {} is of type {} whilst "
            "right-hand side is of type {}",
            stmt->identifier,
            core::primitiveToString(&leftType),
            core::primitiveToString(&rightType)
        );
    }
}

void AnalysisVisitor::visit(core::VariableDecl *stmt) {
    stmt->type->accept(this);
    auto leftType{mReturn};

    Environment *env = mEnvStack.currentEnv();

    if (env->findSymbol(stmt->identifier).has_value()) {
        error(
            stmt->position,
            "redeclaration of {}",
            stmt->identifier
        );
    }

    Environment *enclosingEnv = env->getEnclosing();

    for (;;) {
        if (enclosingEnv == nullptr)
            break;

        std::optional<Symbol> identifierSymbol =
            enclosingEnv->findSymbol(stmt->identifier);

        if (identifierSymbol.has_value() &&
            identifierSymbol->is<FunctionSymbol>()) {
            error(
                stmt->position,
                "redeclaration of {}(...) as a "
                "variable",
                stmt->identifier
            );
        }

        enclosingEnv = enclosingEnv->getEnclosing();
    }

    env->addSymbol(stmt->identifier, {leftType});

    stmt->expr->accept(this);
    auto rightType{mReturn};

    if (leftType != rightType) {
        error(
            stmt->position,
            "left-hand side of {} is of type {} whilst "
            "right-hand side is of type {}",
            stmt->identifier,
            core::primitiveToString(&leftType),
            core::primitiveToString(&rightType)
        );
    }
}

void AnalysisVisitor::visit(core::Block *block) {
    mEnvStack.pushEnv().setType(Environment::Type::BLOCK);

    for (auto &stmt : block->stmts) {
        try {
            stmt->accept(this);
        } catch (SyncAnalysis &) {
            // noop
        }
    }

    mEnvStack.popEnv();
}

void AnalysisVisitor::visit(core::IfStmt *stmt) {
    mEnvStack.pushEnv().setType(Environment::Type::IF);

    try {
        stmt->cond->accept(this);
        auto condType{mReturn};

        if (condType != core::Primitive{core::Base::BOOL}) {
            error(
                stmt->position,
                "if expects boolean condition"
            );
        }
    } catch (SyncAnalysis &) {
        // noop
    }

    stmt->thenBlock->accept(this);

    mEnvStack.popEnv();

    if (stmt->elseBlock) {
        mEnvStack.pushEnv().setType(Environment::Type::ELSE
        );

        stmt->elseBlock->accept(this);

        mEnvStack.popEnv();
    }
}

void AnalysisVisitor::visit(core::ForStmt *stmt) {
    mEnvStack.pushEnv().setType(Environment::Type::FOR);

    try {
        if (stmt->decl) {
            stmt->decl->accept(this);
        }

        stmt->cond->accept(this);
        auto condType{mReturn};

        if (condType != core::Primitive{core::Base::BOOL}) {
            error(
                stmt->position,
                "for expects boolean condition"
            );
        }

        if (stmt->assignment) {
            stmt->assignment->accept(this);
        }
    } catch (SyncAnalysis &) {
        // noop
    }

    stmt->block->accept(this);

    mEnvStack.popEnv();
}

void AnalysisVisitor::visit(core::WhileStmt *stmt) {
    mEnvStack.pushEnv().setType(Environment::Type::WHILE);

    try {
        stmt->cond->accept(this);
        auto condType{mReturn};

        if (condType != core::Primitive{core::Base::BOOL}) {
            error(
                stmt->position,
                "while expects boolean condition"
            );
        }
    } catch (SyncAnalysis &) {
        // noop
    }

    stmt->block->accept(this);

    mEnvStack.popEnv();
}

void AnalysisVisitor::visit(core::ReturnStmt *stmt) {
    stmt->expr->accept(this);
    auto exprType{mReturn};

    std::optional<Environment *> optEnv =
        findEnclosingEnv(Environment::Type::FUNCTION);

    if (!optEnv.has_value()) {
        error(
            stmt->position,
            "return statement must be within a "
            "function block"
        );
    }

    Environment *env = *optEnv;

    std::string enclosingFunc = env->getName().value();

    auto funcSymbol = env->getEnclosing()
                          ->findSymbol(enclosingFunc)
                          ->as<FunctionSymbol>();

    if (exprType != funcSymbol.returnType) {
        error(
            stmt->position,
            "incorrect return type in function {}",
            enclosingFunc
        );
    }
}

void AnalysisVisitor::visit(core::FormalParam *param) {
    param->type->accept(this);
    auto type{mReturn};

    Environment *env = mEnvStack.currentEnv();

    if (env->findSymbol(param->identifier).has_value()) {
        error(
            param->position,
            "redeclaration of {}",
            param->identifier
        );
    }

    Environment *enclosingEnv = env->getEnclosing();

    for (;;) {
        if (enclosingEnv == nullptr)
            break;

        std::optional<Symbol> identifierSignature =
            enclosingEnv->findSymbol(param->identifier);

        if (identifierSignature.has_value() &&
            identifierSignature->is<FunctionSymbol>()) {
            error(
                param->position,
                "redeclaration of {}(...) as a "
                "parameter",
                param->identifier
            );
        }

        enclosingEnv = enclosingEnv->getEnclosing();
    }

    env->addSymbol(param->identifier, {type});
}

void AnalysisVisitor::registerFunction(
    core::FunctionDecl *stmt
) {
    core::abort_if(
        !mEnvStack.isCurrentEnvGlobal(),
        "registerFunction can only be called in "
        "visit(Program *)"
    );

    std::vector<core::Primitive> paramTypes{
        stmt->params.size()
    };

    for (size_t i = 0; i < paramTypes.size(); i++) {
        stmt->params[i]->type->accept(this);
        paramTypes[i] = mReturn;
    }

    stmt->type->accept(this);
    auto returnType{mReturn};

    Symbol signature =
        FunctionSymbol{std::move(paramTypes), returnType};

    Environment *env = mEnvStack.currentEnv();

    if (env->findSymbol(stmt->identifier).has_value()) {
        error(
            stmt->position,
            "redeclaration of {}",
            stmt->identifier
        );
    }

    env->addSymbol(stmt->identifier, signature);

    if (stmt->identifier == "main") {
        error(
            stmt->position,
            "a main function cannot exist",
            stmt->identifier
        );
    }
}

void AnalysisVisitor::visit(core::FunctionDecl *stmt) {
    if (!mEnvStack.isCurrentEnvGlobal()) {
        error(
            stmt->position,
            "function declaration {}(...) is not "
            "allowed "
            "here",
            stmt->identifier
        );
    }

    mEnvStack.pushEnv()
        .setType(Environment::Type::FUNCTION)
        .setName(stmt->identifier);

    for (auto &param : stmt->params) {
        try {
            param->accept(this);
        } catch (SyncAnalysis &) {
            // noop
        }
    }

    stmt->block->accept(this);

    mEnvStack.popEnv();
}

void AnalysisVisitor::visit(core::Program *prog) {
    // HACK: this is piece of code to support
    // mutually exclusive recursion such as in test45.parl
    for (auto &stmt : prog->stmts) {
        if (isFunction.check(stmt.get())) {
            try {
                registerFunction(
                    dynamic_cast<core::FunctionDecl *>(
                        stmt.get()
                    )
                );
            } catch (SyncAnalysis &) {
                // noop
            }
        }
    }

    for (auto &stmt : prog->stmts) {
        try {
            stmt->accept(this);
        } catch (SyncAnalysis &) {
            // noop
        }
    }
}

void AnalysisVisitor::isViableCast(
    core::Primitive &from,
    core::Primitive &to

) {
    const core::Primitive *lPtr = &from;
    const core::Primitive *rPtr = &to;

    while (lPtr != nullptr && rPtr != nullptr) {
        if (lPtr->is<core::Base>() &&
            rPtr->is<core::Array>()) {
            error(
                mPosition,
                "primitive cannot be cast to an array"
            );
        }

        if (lPtr->is<core::Array>() &&
            rPtr->is<core::Base>()) {
            error(
                mPosition,
                "array cannot be cast to a primitive"
            );
        }

        if (lPtr->is<core::Array>() &&
            rPtr->is<core::Array>()) {
            size_t lSize = lPtr->as<core::Array>().size;
            size_t rSize = rPtr->as<core::Array>().size;

            lPtr = lPtr->as<core::Array>().type.get();
            rPtr = rPtr->as<core::Array>().type.get();

            if (lSize != rSize) {
                error(
                    mPosition,
                    "array of size {} cannot be cast to an "
                    "array of size {}",
                    lSize,
                    rSize
                );
            }

            continue;
        }

        return;
    }

    core::abort("unreachable");
}

std::optional<Symbol> AnalysisVisitor::findSymbol(
    const std::string &identifier,
    Environment *stoppingEnv
) {
    auto *env = mEnvStack.currentEnv();

    std::optional<Symbol> symbol{};

    for (;;) {
        symbol = env->findSymbol(identifier);

        if (symbol.has_value() || env == stoppingEnv)
            break;

        env = env->getEnclosing();
    }

    return symbol;
}

std::optional<Environment *>
AnalysisVisitor::findEnclosingEnv(Environment::Type envType
) {
    auto *env = mEnvStack.currentEnv();

    while (!env->isGlobal() && env->getType() != envType) {
        env = env->getEnclosing();
    }

    if (env->getType() == envType) {
        return {env};
    }

    return {};
}

bool AnalysisVisitor::hasError() const {
    return mHasError;
}

std::unique_ptr<Environment>
AnalysisVisitor::getEnvironment() {
    return mEnvStack.extractGlobal();
}

void AnalysisVisitor::analyse(core::Program *prog) {
    prog->accept(this);

    ReturnVisitor returns{};

    prog->accept(&returns);

    if (returns.hasError()) {
        mHasError = true;
    }
}

void AnalysisVisitor::reset() {
    isFunction.reset();
    mHasError = false;
    mPosition = {0, 0};
    mReturn = core::Primitive{};
    mEnvStack = {};
}

}  // namespace PArL
