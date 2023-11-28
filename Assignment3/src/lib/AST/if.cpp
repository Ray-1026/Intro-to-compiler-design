#include "AST/if.hpp"

// TODO
IfNode::IfNode(const uint32_t line, const uint32_t col, AstNode *const p_expr, AstNode *const p_compound_stmt1,
               AstNode *const p_compound_stmt2)
    : AstNode{line, col}, expr(p_expr), compound_stmt1(p_compound_stmt1), compound_stmt2(p_compound_stmt2)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void IfNode::print() {}

void IfNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    expr->accept(p_visitor);
    compound_stmt1->accept(p_visitor);
    if (compound_stmt2 != NULL)
        compound_stmt2->accept(p_visitor);
}
