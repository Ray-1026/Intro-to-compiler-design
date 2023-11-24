#ifndef __AST_IF_NODE_H
#define __AST_IF_NODE_H

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"

class IfNode : public AstNode {
  public:
    IfNode(const uint32_t line, const uint32_t col,
           /* TODO: expression, compound statement, compound statement */
           AstNode *const p_expr, AstNode *const p_compound_stmt1, AstNode *const p_compound_stmt2);
    ~IfNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    // TODO: expression, compound statement, compound statement
    AstNode *expr, *compound_stmt1, *compound_stmt2;
};

#endif
