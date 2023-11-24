#ifndef __AST_DECL_NODE_H
#define __AST_DECL_NODE_H

#include "AST/ast.hpp"
#include "AST/variable.hpp"
#include "visitor/AstNodeVisitor.hpp"

class DeclNode : public AstNode {
  public:
    // variable declaration
    DeclNode(const uint32_t line, const uint32_t col,
             /* TODO: identifiers, type */
             std::vector<VariableNode *> *const variable_list);

    // constant variable declaration
    // DeclNode(const uint32_t, const uint32_t col
    //         /* TODO: identifiers, constant */);

    ~DeclNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    std::vector<const char *> returnType();

  private:
    // TODO: variables
    std::vector<VariableNode *> m_variables;
};

#endif
