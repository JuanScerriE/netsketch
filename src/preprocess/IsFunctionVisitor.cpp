// parl
#include <preprocess/IsFunctionVisitor.hpp>

namespace PArL {

void IsFunctionVisitor::visit(core::Type *) {
}

void IsFunctionVisitor::visit(core::Expr *) {
}

void IsFunctionVisitor::visit(core::PadWidth *) {
}

void IsFunctionVisitor::visit(core::PadHeight *) {
}

void IsFunctionVisitor::visit(core::PadRead *) {
}

void IsFunctionVisitor::visit(core::PadRandomInt *) {
}

void IsFunctionVisitor::visit(core::BooleanLiteral *) {
}

void IsFunctionVisitor::visit(core::IntegerLiteral *) {
}

void IsFunctionVisitor::visit(core::FloatLiteral *) {
}

void IsFunctionVisitor::visit(core::ColorLiteral *) {
}

void IsFunctionVisitor::visit(core::ArrayLiteral *) {
}

void IsFunctionVisitor::visit(core::Variable *) {
}

void IsFunctionVisitor::visit(core::ArrayAccess *) {
}

void IsFunctionVisitor::visit(core::FunctionCall *) {
}

void IsFunctionVisitor::visit(core::SubExpr *) {
}

void IsFunctionVisitor::visit(core::Binary *) {
}

void IsFunctionVisitor::visit(core::Unary *) {
}

void IsFunctionVisitor::visit(core::Assignment *) {
}

void IsFunctionVisitor::visit(core::VariableDecl *) {
}

void IsFunctionVisitor::visit(core::PrintStmt *) {
}

void IsFunctionVisitor::visit(core::DelayStmt *) {
}

void IsFunctionVisitor::visit(core::WriteBoxStmt *) {
}

void IsFunctionVisitor::visit(core::WriteStmt *) {
}

void IsFunctionVisitor::visit(core::ClearStmt *) {
}

void IsFunctionVisitor::visit(core::Block *) {
}

void IsFunctionVisitor::visit(core::FormalParam *) {
}

void IsFunctionVisitor::visit(core::FunctionDecl *) {
    mReturn = true;
}

void IsFunctionVisitor::visit(core::IfStmt *) {
}

void IsFunctionVisitor::visit(core::ForStmt *) {
}

void IsFunctionVisitor::visit(core::WhileStmt *) {
}

void IsFunctionVisitor::visit(core::ReturnStmt *) {
}

void IsFunctionVisitor::visit(core::Program *) {
}

void IsFunctionVisitor::reset() {
    mReturn = false;
}

bool IsFunctionVisitor::check(core::Node *node) {
    node->accept(this);

    bool result = mReturn;

    reset();

    return result;
}

}  // namespace PArL
