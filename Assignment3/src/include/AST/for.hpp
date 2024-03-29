#ifndef __AST_FOR_NODE_H
#define __AST_FOR_NODE_H

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"

class ForNode : public AstNode {
  public:
    ForNode(const uint32_t line, const uint32_t col,
            /* TODO: declaration, assignment, expression,
             *       compound statement */
            AstNode *const p_decl, AstNode *const p_assign, AstNode *const p_const_val, AstNode *const p_compound_stmt);
    ~ForNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

  private:
    // TODO: declaration, assignment, expression, compound statement
    AstNode *m_decl, *m_assign, *m_const_val, *m_compound_stmt;
};

#endif
