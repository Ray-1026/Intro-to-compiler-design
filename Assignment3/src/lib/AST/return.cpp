#include "AST/return.hpp"

// TODO
ReturnNode::ReturnNode(const uint32_t line, const uint32_t col, AstNode *const expression)
    : AstNode{line, col}, expr(expression)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void ReturnNode::print() {}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    expr->accept(p_visitor);
}
