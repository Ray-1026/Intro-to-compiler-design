#ifndef __AST_FUNCTION_INVOCATION_NODE_H
#define __AST_FUNCTION_INVOCATION_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

class FunctionInvocationNode : public ExpressionNode {
  public:
    FunctionInvocationNode(const uint32_t line, const uint32_t col,
                           /* TODO: function name, expressions */
                           std::string func_name, std::vector<AstNode *> *const expressions);
    ~FunctionInvocationNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *getFuncName() { return name.c_str(); }

  private:
    // TODO: function name, expressions
    std::string name;
    std::vector<AstNode *> *exprs;
};

#endif
