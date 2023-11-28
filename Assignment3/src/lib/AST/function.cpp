#include "AST/function.hpp"

// TODO
FunctionNode::FunctionNode(const uint32_t line, const uint32_t col, std::string p_name,
                           std::vector<DeclNode *> *const p_decl, std::string p_type, AstNode *const p_compound)
    : AstNode{line, col}, name(p_name), type(p_type), m_decl(p_decl), m_compound(p_compound)
{
    proto = p_type + " (";
    if (m_decl != nullptr) {
        for (unsigned int i = 0; i < m_decl->size(); i++) {
            std::vector<const char *> all_type = m_decl->at(i)->returnType();
            for (unsigned int j = 0; j < all_type.size(); j++) {
                proto += all_type[j];
                if (j != all_type.size() - 1)
                    proto += ", ";
            }
            if (i != m_decl->size() - 1)
                proto += ", ";
        }
    }
    proto += ")";
}

// TODO: You may use code snippets in AstDumper.cpp
void FunctionNode::print() {}

void FunctionNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    if (m_decl != nullptr) {
        for (auto &decl : *m_decl) {
            decl->accept(p_visitor);
        }
    }

    if (m_compound != nullptr)
        m_compound->accept(p_visitor);
}
