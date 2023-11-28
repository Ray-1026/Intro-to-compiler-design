#include "AST/CompoundStatement.hpp"

// TODO
CompoundStatementNode::CompoundStatementNode(const uint32_t line, const uint32_t col,
                                             std::vector<AstNode *> *const p_declarations,
                                             std::vector<AstNode *> *const p_statements)
    : AstNode{line, col}, m_declarations(p_declarations), m_statements(p_statements)
{
}

// TODO: You may use code snippets in AstDumper.cpp
void CompoundStatementNode::print() {}

void CompoundStatementNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    if (m_declarations != NULL) {
        for (auto &declaration : *m_declarations)
            declaration->accept(p_visitor);
    }

    if (m_statements != NULL) {
        for (auto &statement : *m_statements)
            statement->accept(p_visitor);
    }
}
