#include "AST/decl.hpp"

// TODO
DeclNode::DeclNode(const uint32_t line, const uint32_t col, std::vector<VariableNode *> *const variable_list)
    : AstNode{line, col}, m_variables(*variable_list)
{
}

// TODO
// DeclNode::DeclNode(const uint32_t line, const uint32_t col)
//    : AstNode{line, col} {}

// TODO: You may use code snippets in AstDumper.cpp
void DeclNode::print() {}

void DeclNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    // TODO
    for (auto &var : m_variables)
        var->accept(p_visitor);
}

std::vector<const char *> DeclNode::returnType()
{
    std::vector<const char *> all_type;
    for (auto &var : m_variables)
        all_type.push_back(var->getVarType());

    return all_type;
}