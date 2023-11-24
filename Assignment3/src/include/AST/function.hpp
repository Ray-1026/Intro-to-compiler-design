#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "visitor/AstNodeVisitor.hpp"

class FunctionNode : public AstNode {
  public:
    FunctionNode(const uint32_t line, const uint32_t col,
                 /* TODO: name, declarations, return type,
                  *       compound statement (optional) */
                 std::string p_name, std::vector<DeclNode *> *const p_decl, std::string p_type,
                 AstNode *const p_compound);
    ~FunctionNode() = default;

    void print() override;
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;

    const char *funcName() { return name.c_str(); };
    const char *funcProto() { return proto.c_str(); };

  private:
    // TODO: name, declarations, return type, compound statement
    std::string name, type, proto;
    std::vector<DeclNode *> *m_decl;
    AstNode *m_compound;
};

#endif
