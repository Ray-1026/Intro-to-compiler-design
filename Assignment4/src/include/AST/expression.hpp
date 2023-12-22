#ifndef AST_EXPRESSION_NODE_H
#define AST_EXPRESSION_NODE_H

#include "AST/ast.hpp"
#include "PType.hpp"

class ExpressionNode : public AstNode {
  public:
    ~ExpressionNode() = default;
    ExpressionNode(const uint32_t line, const uint32_t col) : AstNode{line, col} {}

    const char *getPTypeCString() { return (m_type == nullptr) ? "null" : m_type->getPTypeCString(); }
    int getDimension() { return m_type->getDimension(); }

    void setType(std::string type);
    void setDim(std::vector<uint64_t> &dim) { m_type->setDimensions(dim); }

  protected:
    // for carrying type of result of an expression
    // TODO: for next assignment
    PTypeSharedPtr m_type = nullptr;
};

#endif
