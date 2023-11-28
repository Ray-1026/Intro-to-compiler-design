#include "AST/assignment.hpp"

// TODO
AssignmentNode::AssignmentNode(const uint32_t line, const uint32_t col, AstNode *const p_var_ref, AstNode *const p_expr)
    : AstNode{line, col}, var_ref(p_var_ref), expr(p_expr)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void AssignmentNode::print() {}

void AssignmentNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    var_ref->accept(p_visitor);
    expr->accept(p_visitor);
}
