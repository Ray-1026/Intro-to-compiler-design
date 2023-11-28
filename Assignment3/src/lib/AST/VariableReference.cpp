#include "AST/VariableReference.hpp"

// TODO
VariableReferenceNode::VariableReferenceNode(const uint32_t line, const uint32_t col, std::string name,
                                             std::vector<AstNode *> *const p_expressions)
    : ExpressionNode{line, col}, name(name), expressions(p_expressions)
{
}

// TODO
// VariableReferenceNode::VariableReferenceNode(const uint32_t line,
//                                              const uint32_t col)
//     : ExpressionNode{line, col} {}

// TODO: You may use code snippets in AstDumper.cpp
void VariableReferenceNode::print() {}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    if (expressions != NULL) {
        for (auto &expr : *expressions) {
            expr->accept(p_visitor);
        }
    }
}
