// parl
#include <parl/AST.hpp>
#include <parser/PrinterVisitor.hpp>

namespace PArL {

void PrinterVisitor::visit(core::Type *type) {
    std::string primitive = baseToString(type->base);
    if (type->isArray) {
        if (type->size) {
            print_with_tabs(
                "{}[{}]",
                primitive,
                type->size->value
            );
        } else {
            print_with_tabs("{}[]", primitive);
        }
    } else {
        print_with_tabs("{}", primitive);
    }
}

void PrinterVisitor::visit(core::Expr *expr) {
    if (expr->type.has_value()) {
        print_with_tabs("as");
        mTabCount++;
        (*expr->type)->accept(this);
        mTabCount--;
    }
}

void PrinterVisitor::visit(core::PadWidth *expr) {
    print_with_tabs("__width");

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::PadHeight *expr) {
    print_with_tabs("__height");

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::PadRead *expr) {
    print_with_tabs("__read =>");
    mTabCount++;
    expr->x->accept(this);
    expr->y->accept(this);
    mTabCount--;

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::PadRandomInt *expr) {
    print_with_tabs("__random_int =>");

    mTabCount++;
    expr->max->accept(this);
    mTabCount--;

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::BooleanLiteral *literal) {
    print_with_tabs("bool {}", literal->value);

    literal->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::IntegerLiteral *literal) {
    print_with_tabs("int {}", literal->value);

    literal->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::FloatLiteral *literal) {
    print_with_tabs("float {}", literal->value);

    literal->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::ColorLiteral *literal) {
    print_with_tabs(
        "color #{:x}{:x}{:x}",
        literal->value.r(),
        literal->value.g(),
        literal->value.b()
    );

    literal->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::ArrayLiteral *literal) {
    print_with_tabs("[");

    mTabCount++;
    for (auto &param : literal->exprs) {
        param->accept(this);
    }
    mTabCount--;

    print_with_tabs("]");

    literal->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::Variable *expr) {
    print_with_tabs("Variable {}", expr->identifier);

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::ArrayAccess *expr) {
    print_with_tabs("Array Access {} =>", expr->identifier);

    mTabCount++;
    expr->index->accept(this);
    mTabCount--;

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::FunctionCall *expr) {
    print_with_tabs(
        "Function Call {} =>",
        expr->identifier
    );
    mTabCount++;
    for (auto &param : expr->params) {
        param->accept(this);
    }
    mTabCount--;

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::SubExpr *expr) {
    expr->subExpr->accept(this);
}

void PrinterVisitor::visit(core::Binary *expr) {
    print_with_tabs(
        "Binary Operation {} =>",
        operationToString(expr->op)
    );
    mTabCount++;
    expr->left->accept(this);
    expr->right->accept(this);
    mTabCount--;

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::Unary *expr) {
    print_with_tabs(
        "Unary Operation {} =>",
        operationToString(expr->op)
    );
    mTabCount++;
    expr->expr->accept(this);
    mTabCount--;

    expr->core::Expr::accept(this);
}

void PrinterVisitor::visit(core::Assignment *stmt) {
    print_with_tabs("Assign {} =>", stmt->identifier);

    if (stmt->index) {
        mTabCount++;
        print_with_tabs("[");
        stmt->index->accept(this);
        print_with_tabs("]");
        mTabCount--;
    }

    mTabCount++;
    stmt->expr->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::VariableDecl *stmt) {
    print_with_tabs("let {} : ", stmt->identifier);

    mTabCount++;
    stmt->type->accept(this);
    stmt->expr->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::PrintStmt *stmt) {
    print_with_tabs("__print =>");

    mTabCount++;
    stmt->expr->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::DelayStmt *stmt) {
    print_with_tabs("__delay =>");

    mTabCount++;
    stmt->expr->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::ClearStmt *stmt) {
    print_with_tabs("__clear =>");

    mTabCount++;
    stmt->color->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::WriteBoxStmt *stmt) {
    print_with_tabs("__write_box =>");

    mTabCount++;
    stmt->x->accept(this);
    stmt->y->accept(this);
    stmt->w->accept(this);
    stmt->h->accept(this);
    stmt->color->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::WriteStmt *stmt) {
    print_with_tabs("__write =>");

    mTabCount++;
    stmt->x->accept(this);
    stmt->y->accept(this);
    stmt->color->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::Block *stmt) {
    print_with_tabs("{{");

    mTabCount++;
    for (auto &innerStmt : stmt->stmts) {
        innerStmt->accept(this);
    }
    mTabCount--;

    print_with_tabs("}}");
}

void PrinterVisitor::visit(core::IfStmt *stmt) {
    print_with_tabs("If =>");
    mTabCount++;
    stmt->cond->accept(this);
    stmt->thenBlock->accept(this);
    mTabCount--;

    if (stmt->elseBlock) {
        print_with_tabs("Else =>");
        mTabCount++;
        stmt->elseBlock->accept(this);
        mTabCount--;
    }
}

void PrinterVisitor::visit(core::ForStmt *stmt) {
    print_with_tabs("For =>");

    mTabCount++;
    if (stmt->decl) {
        stmt->decl->accept(this);
    }
    stmt->cond->accept(this);
    if (stmt->assignment) {
        stmt->assignment->accept(this);
    }
    stmt->block->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::WhileStmt *stmt) {
    print_with_tabs("While =>");

    mTabCount++;
    stmt->cond->accept(this);
    stmt->block->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::ReturnStmt *stmt) {
    print_with_tabs("Return =>");

    mTabCount++;
    stmt->expr->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::FormalParam *param) {
    print_with_tabs(
        "Formal Param {} =>",
        param->identifier
    );

    mTabCount++;
    param->type->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::FunctionDecl *stmt) {
    print_with_tabs("Func Decl {} =>", stmt->identifier);
    mTabCount++;
    for (auto &param : stmt->params) {
        param->accept(this);
    }
    stmt->type->accept(this);
    stmt->block->accept(this);
    mTabCount--;
}

void PrinterVisitor::visit(core::Program *prog) {
    print_with_tabs("Program =>");

    mTabCount++;
    for (auto &stmt : prog->stmts) {
        stmt->accept(this);
    }
    mTabCount--;
}

void PrinterVisitor::reset() {
    mTabCount = 0;
}

}  // namespace PArL
