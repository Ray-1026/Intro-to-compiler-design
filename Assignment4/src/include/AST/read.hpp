#ifndef AST_READ_NODE_H
#define AST_READ_NODE_H

#include "AST/VariableReference.hpp"
#include "AST/ast.hpp"

#include <memory>

class ReadNode final : public AstNode {
  private:
    std::unique_ptr<VariableReferenceNode> m_target;

  public:
    ~ReadNode() = default;
    ReadNode(const uint32_t line, const uint32_t col, VariableReferenceNode *p_target)
        : AstNode{line, col}, m_target(p_target)
    {
    }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    std::string getTargetCString() const { return m_target->getPTypeCString(); }
    std::string getNameCString() const { return m_target->getNameCString(); }
    int getDimension() { return m_target->getDimension(); }
    int getTargetLocationLine() const { return m_target->getLocation().line; }
    int getTargetLocationCol() const { return m_target->getLocation().col; }

    bool checkChlid() { return (strcmp(m_target->getPTypeCString(), "null")) ? true : false; }
};

#endif
