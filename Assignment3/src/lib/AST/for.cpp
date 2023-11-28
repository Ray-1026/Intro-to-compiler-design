#include "AST/for.hpp"

// TODO
ForNode::ForNode(const uint32_t line, const uint32_t col, AstNode *const p_decl, AstNode *const p_assign,
                 AstNode *const p_const_val, AstNode *const p_compound_stmt)
    : AstNode{line, col}, m_decl(p_decl), m_assign(p_assign), m_const_val(p_const_val), m_compound_stmt(p_compound_stmt)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void ForNode::print() {}

void ForNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    m_decl->accept(p_visitor);
    m_assign->accept(p_visitor);
    m_const_val->accept(p_visitor);
    m_compound_stmt->accept(p_visitor);
}
