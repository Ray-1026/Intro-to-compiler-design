#include "AST/VariableReference.hpp"

#include <algorithm>

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(p_visitor); };

    for_each(m_indices.begin(), m_indices.end(), visit_ast_node);
}

ExpressionNode *VariableReferenceNode::errorIndex()
{
    for (auto &index : m_indices) {
        if (strcmp(index->getPTypeCString(), "integer") != 0)
            return index.get();
    }
    return nullptr;
}

void VariableReferenceNode::updateDimension(std::string type)
{
    std::vector<uint64_t> dim;
    for (size_t i = 0; i < type.length(); i++) {
        if (type[i] == '[') {
            int j = i + 1;
            while (type[j] != ']')
                j++;
            dim.emplace_back(std::stoi(type.substr(i + 1, j - i - 1)));
            i = j;
        }
    }
    for (size_t i = 0; i < m_indices.size(); i++)
        dim.erase(dim.begin());

    m_type->setDimensions(dim);
}