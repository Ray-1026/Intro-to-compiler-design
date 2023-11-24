#ifndef AST_PROGRAM_NODE_H
#define AST_PROGRAM_NODE_H

#include "AST/CompoundStatement.hpp"
#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <memory>

class ProgramNode final : public AstNode {
  private:
    std::string name;
    // TODO: return type, declarations, functions, compound statement
    std::vector<AstNode *> *m_decl, *m_func;
    AstNode *m_compound;

  public:
    ~ProgramNode() = default;
    ProgramNode(const uint32_t line, const uint32_t col, const char *const p_name,
                /* TODO: return type, declarations, functions,
                 *       compound statement */
                std::vector<AstNode *> *const p_decl, std::vector<AstNode *> *const p_func, AstNode *const p_compound);

    // visitor pattern version: const char *getNameCString() const;
    void print() override;
    const char *getNameCString() const { return name.c_str(); }
    void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
    void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
