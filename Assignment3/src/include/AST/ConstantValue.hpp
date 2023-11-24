#ifndef __AST_CONSTANT_VALUE_NODE_H
#define __AST_CONSTANT_VALUE_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

class ConstantValueNode : public ExpressionNode {
  public:
    ConstantValueNode(const uint32_t line, const uint32_t col,
                      /* TODO: constant value */
                      const char *const_value);
    ~ConstantValueNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };

    const char *getVal();

  private:
    // TODO: constant value
    std::string value;
};

#endif
