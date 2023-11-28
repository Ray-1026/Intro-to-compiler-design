#include "AST/program.hpp"

// TODO
ProgramNode::ProgramNode(const uint32_t line, const uint32_t col, const char *const p_name,
                         std::vector<AstNode *> *const p_decl, std::vector<AstNode *> *const p_func,
                         AstNode *const p_compound)
    : AstNode{line, col}, name(p_name), m_decl(p_decl), m_func(p_func), m_compound(p_compound)
{
}

// visitor pattern version: const char *ProgramNode::getNameCString() const { return name.c_str(); }

void ProgramNode::print()
{
    // TODO
    // outputIndentationSpace();

    std::printf("program <line: %u, col: %u> %s %s\n", location.line, location.col, name.c_str(), "void");

    // TODO
    // incrementIndentation();
    // visitChildNodes();
    // decrementIndentation();
}

void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor)
{
    if (m_decl != NULL) {
        for (auto &decl : *m_decl) {
            decl->accept(p_visitor);
        }
    }

    if (m_func != NULL) {
        for (auto &func : *m_func) {
            func->accept(p_visitor);
        }
    }

    m_compound->accept(p_visitor);
}
