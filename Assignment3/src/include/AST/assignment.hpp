#ifndef __AST_ASSIGNMENT_NODE_H
#define __AST_ASSIGNMENT_NODE_H

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"

class AssignmentNode : public AstNode {
  public:
    AssignmentNode(const uint32_t line, const uint32_t col,
                   /* TODO: variable reference, expression */
                   AstNode *const p_var_ref, AstNode *const p_expr);
    ~AssignmentNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    // TODO: variable reference, expression
    AstNode *var_ref, *expr;
};

#endif
