#ifndef AST_ASSIGNMENT_NODE_H
#define AST_ASSIGNMENT_NODE_H

#include "AST/VariableReference.hpp"
#include "AST/ast.hpp"
#include "AST/expression.hpp"

#include <memory>

class AssignmentNode final : public AstNode {
  private:
    std::unique_ptr<VariableReferenceNode> m_lvalue;
    std::unique_ptr<ExpressionNode> m_expr;

  public:
    ~AssignmentNode() = default;
    AssignmentNode(const uint32_t line, const uint32_t col, VariableReferenceNode *p_var_ref, ExpressionNode *p_expr)
        : AstNode{line, col}, m_lvalue(p_var_ref), m_expr(p_expr)
    {
    }

    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); }
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    VariableReferenceNode *getLValue() { return m_lvalue.get(); }
    const char *getLValueCString() { return m_lvalue->getPTypeCString(); }
    int getLValueDimension() { return m_lvalue->getDimension(); }

    ExpressionNode *getExpr() { return m_expr.get(); }
    const char *getExprCString() { return m_expr->getPTypeCString(); }
};

#endif
