#ifndef AST_PRINT_NODE_H
#define AST_PRINT_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string.h>

class PrintNode final : public AstNode {
  private:
    std::unique_ptr<ExpressionNode> m_target;

  public:
    ~PrintNode() = default;
    PrintNode(const uint32_t line, const uint32_t col, ExpressionNode *p_target)
        : AstNode{line, col}, m_target(p_target)
    {
    }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    std::string getTargetCString() const { return m_target->getPTypeCString(); }
    int getDimension() { return m_target->getDimension(); }
    int getTargetLocationLine() const { return m_target->getLocation().line; }
    int getTargetLocationCol() const { return m_target->getLocation().col; }

    bool checkChlid() { return (strcmp(m_target->getPTypeCString(), "null")) ? true : false; }
};

#endif
