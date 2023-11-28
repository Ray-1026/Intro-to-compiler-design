# hw3 report

|||
|-:|:-|
|Name|蔡師睿|
|ID|110550093|

## How much time did you spend on this project

> About 14~15 hours to finish the whole assignment.

## Project overview

### AST
For all the classes, include the header first to use visitor pattern.
```cpp
#include "visitor/AstNodeVisitor.hpp"
```
Next, modify the constructor to allow the proper parameters to pass in for each classese, and then, add the following member functions.
```cpp
void accept(AstNodeVisitor &p_visitor) override { p_visitor.visit(*this); };
void visitChildNodes(AstNodeVisitor &p_visitor) override;
```
These functions allow using visitor pattern. Function `accept` is for the visitor to visit the node itself, and function `visitChildNodes` is to traverse the AST recursively and dump every nodes in AST.

Additionally, I implement other functions in some classes to get information of its private members. For example, return variable type, variable name, or operator.

### scanner.I
Use the provided scanner, but there is something to modify to meet the requirement designed in parser. We have to change the data type and pass the value through `yylval`.

```cpp
/* INT_LITERAL */
yylval.integer = strtol(yytext, NULL, 10)
yylval.integer = strtol(yytext, NULL, 8) // oct

/* REAL_LITERAL */
yylval.real = strtod(yytext, NULL)

/* STRING_LITERAL */
yylval.identifier = strndup(string_literal, MAX_ID_LENG);
```

### parser.y
In order to pass the varible's type, value, line, and column, I define a struct called *Variable* to store them.
```cpp
%code requires {
    #include "AST/ast.hpp"

    struct Variable {
        std::string type, val;
        size_t line, column;

        Variable(std::string type, std::string val) : type(type), val(val) {}
        Variable(std::string val, size_t line, size_t column) : type("ID"), val(val), line(line), column(column) {}
    };
}
```
The `%union` is to allocate memory for its members.
```cpp
%union {
    /* basic semantic value */
    char *identifier;
    int integer;
    double real;

    Variable *variable;
    AstNode *node;

    std::vector<AstNode *> *node_list;
    std::vector<Variable *> *var_list;
};
```
The `%type` declaration is to declare a type for each production.
```cpp
%type <identifier> ProgramName ID STRING_LITERAL Type ScalarType ArrType ArrDecl IntegerAndReal ReturnType FunctionName
%type <integer> NegOrNot INT_LITERAL
%type <real> REAL_LITERAL
%type <variable> LiteralConstant StringAndBoolean 
%type <node> CompoundStatement Declaration FormalArg FunctionDeclaration FunctionDefinition Function Statement ProgramUnit Program
%type <node> Expression Simple FunctionInvocation VariableReference ElseOrNot Condition While For Return FunctionCall
%type <node_list> DeclarationList Declarations FunctionList Functions FormalArgs FormalArgList 
%type <node_list> Statements StatementList Expressions ExpressionList ArrRefs ArrRefList
%type <var_list> IdList
```
Afterward, we can verify that AST construction against our designated design rules. Additional code details can be found in the `parser.y` file.

## What is the hardest you think in this project

I think that the hardest part is to understand what AST and visitor pattern is and how they works. Before starting, you have to design your compiler carefully or the process can become chaotic. Once the foundational aspects are in place and the design is robust, it'll become easier to add additional functions to complete it.

## Feedback to T.A.s

A good practice in OOP.
