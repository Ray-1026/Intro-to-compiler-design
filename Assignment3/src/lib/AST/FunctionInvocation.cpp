#include "AST/FunctionInvocation.hpp"

// TODO
FunctionInvocationNode::FunctionInvocationNode(const uint32_t line, const uint32_t col, std::string func_name,
                                               std::vector<AstNode *> *const expressions)
    : ExpressionNode{line, col}, name(func_name), exprs(expressions)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void FunctionInvocationNode::print() {}

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    if (exprs != NULL) {
        for (auto &expr : *exprs) {
            expr->accept(p_visitor);
        }
    }
}
