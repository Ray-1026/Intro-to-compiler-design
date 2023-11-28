#include "AST/BinaryOperator.hpp"

// TODO
BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col, std::string op, AstNode *const lhs,
                                       AstNode *const rhs)
    : ExpressionNode{line, col}, op(op), lhs(lhs), rhs(rhs)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void BinaryOperatorNode::print() {}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    lhs->accept(p_visitor);
    rhs->accept(p_visitor);
}
