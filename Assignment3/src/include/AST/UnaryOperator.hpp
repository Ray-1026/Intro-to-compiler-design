#ifndef __AST_UNARY_OPERATOR_NODE_H
#define __AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

class UnaryOperatorNode : public ExpressionNode {
  public:
    UnaryOperatorNode(const uint32_t line, const uint32_t col,
                      /* TODO: operator, expression */
                      std::string op, AstNode *const exprression);
    ~UnaryOperatorNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *getOp() { return op.c_str(); };

  private:
    // TODO: operator, expression
    std::string op;
    AstNode *expr;
};

#endif
