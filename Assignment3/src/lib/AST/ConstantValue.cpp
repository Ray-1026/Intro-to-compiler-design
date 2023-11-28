#include "AST/ConstantValue.hpp"

// TODO
ConstantValueNode::ConstantValueNode(const uint32_t line, const uint32_t col, const char *const_value)
    : ExpressionNode{line, col}, value(const_value)
{
}

const char *ConstantValueNode::getVal() { return value.c_str(); }

// TODO: You may use code snippets in AstDumper.cpp
void ConstantValueNode::print() {}
