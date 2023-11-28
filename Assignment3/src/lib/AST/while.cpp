#include "AST/while.hpp"

// TODO
WhileNode::WhileNode(const uint32_t line, const uint32_t col, AstNode *const p_expr, AstNode *const p_compound_stmt)
    : AstNode{line, col}, expr(p_expr), compound_stmt(p_compound_stmt)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void WhileNode::print() {}

void WhileNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    expr->accept(p_visitor);
    compound_stmt->accept(p_visitor);
}
