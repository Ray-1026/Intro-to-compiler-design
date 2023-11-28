#include "AST/print.hpp"

// TODO
PrintNode::PrintNode(const uint32_t line, const uint32_t col, AstNode *const exprression)
    : AstNode{line, col}, expr(exprression)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void PrintNode::print() {}

void PrintNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    expr->accept(p_visitor);
}
