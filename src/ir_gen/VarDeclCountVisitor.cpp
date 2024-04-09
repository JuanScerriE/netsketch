// parl
#include <ir_gen/VarDeclCountVisitor.hpp>

namespace PArL {

void VarDeclCountVisitor::visit(core::Type *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::Expr *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::PadWidth *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::PadHeight *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::PadRead *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::PadRandomInt *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::BooleanLiteral *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::IntegerLiteral *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::FloatLiteral *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::ColorLiteral *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::ArrayLiteral *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::Variable *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::ArrayAccess *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::FunctionCall *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::SubExpr *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::Binary *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::Unary *) {
    core::abort("unimplemented");
}

void VarDeclCountVisitor::visit(core::Assignment *) {
}

void VarDeclCountVisitor::visit(core::VariableDecl *stmt) {
    std::optional<Symbol> symbol =
        mEnv->findSymbol(stmt->identifier);

    auto &variable = symbol->asRef<VariableSymbol>();

    mCount += variable.type.is<core::Array>()
                  ? variable.type.as<core::Array>().size
                  : 1;
}

void VarDeclCountVisitor::visit(core::PrintStmt *) {
}

void VarDeclCountVisitor::visit(core::DelayStmt *) {
}

void VarDeclCountVisitor::visit(core::WriteBoxStmt *) {
}

void VarDeclCountVisitor::visit(core::WriteStmt *) {
}

void VarDeclCountVisitor::visit(core::ClearStmt *) {
}

void VarDeclCountVisitor::visit(core::Block *) {
}

void VarDeclCountVisitor::visit(core::FormalParam *param) {
    std::optional<Symbol> symbol =
        mEnv->findSymbol(param->identifier);

    auto &variable = symbol->asRef<VariableSymbol>();

    mCount += variable.type.is<core::Array>()
                  ? variable.type.as<core::Array>().size
                  : 1;
}

void VarDeclCountVisitor::visit(core::FunctionDecl *) {
}

void VarDeclCountVisitor::visit(core::IfStmt *) {
}

void VarDeclCountVisitor::visit(core::ForStmt *) {
}

void VarDeclCountVisitor::visit(core::WhileStmt *) {
}

void VarDeclCountVisitor::visit(core::ReturnStmt *) {
}

void VarDeclCountVisitor::visit(core::Program *) {
}

void VarDeclCountVisitor::reset() {
    mCount = 0;
}

size_t VarDeclCountVisitor::count(
    core::Node *node,
    Environment *env
) {
    mEnv = env;

    node->accept(this);

    size_t result = mCount;

    reset();

    return result;
}

}  // namespace PArL
