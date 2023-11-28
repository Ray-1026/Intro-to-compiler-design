#include "AST/UnaryOperator.hpp"

// TODO
UnaryOperatorNode::UnaryOperatorNode(const uint32_t line, const uint32_t col, std::string op,
                                     AstNode *const exprression)
    : ExpressionNode{line, col}, op(op), expr(exprression)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void UnaryOperatorNode::print() {}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    expr->accept(p_visitor);
}
