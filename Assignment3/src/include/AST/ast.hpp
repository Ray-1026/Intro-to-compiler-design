#ifndef AST_AST_NODE_H
#define AST_AST_NODE_H

#include <cstdint>
#include <string>
#include <vector>

class AstNodeVisitor;

struct Location {
    uint32_t line;
    uint32_t col;

    ~Location() = default;
    Location(const uint32_t line, const uint32_t col) : line(line), col(col) {}
};

class AstNode {
  protected:
    Location location;

  public:
    virtual ~AstNode() = 0;
    AstNode(const uint32_t line, const uint32_t col);

    const Location &getLocation() const;

    virtual void print() = 0;
    virtual void accept(AstNodeVisitor &p_visitor) = 0;
    virtual void visitChildNodes(AstNodeVisitor &p_visitor){};
};

#endif
