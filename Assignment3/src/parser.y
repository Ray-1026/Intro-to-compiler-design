%{
#include "AST/AstDumper.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/expression.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/program.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/variable.hpp"
#include "AST/while.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define YYLTYPE yyltype

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern uint32_t line_num;   /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

static AstNode *root;

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%code requires {
    #include "AST/ast.hpp"

    struct Variable {
        std::string type, val;
        size_t line, column;

        Variable(std::string type, std::string val) : type(type), val(val) {}
        Variable(std::string val, size_t line, size_t column) : type("ID"), val(val), line(line), column(column) {}
    };
}

    /* For yylval */
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

%type <identifier> ProgramName ID STRING_LITERAL Type ScalarType ArrType ArrDecl IntegerAndReal ReturnType FunctionName
%type <integer> NegOrNot INT_LITERAL
%type <real> REAL_LITERAL
%type <variable> LiteralConstant StringAndBoolean 
%type <node> CompoundStatement Declaration FormalArg FunctionDeclaration FunctionDefinition Function Statement ProgramUnit Program
%type <node> Expression Simple FunctionInvocation VariableReference ElseOrNot Condition While For Return FunctionCall
%type <node_list> DeclarationList Declarations FunctionList Functions FormalArgs FormalArgList 
%type <node_list> Statements StatementList Expressions ExpressionList ArrRefs ArrRefList
%type <var_list> IdList

    /* Follow the order in scanner.l */

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
%left OR
%left AND
%right NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN_ /* Use BEGIN_ since BEGIN is a keyword in lex */
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

%%

ProgramUnit:
    Program {
        $$ = $1;
    }
    |
    Function {
        $$ = $1;
    }
;

Program:
    ProgramName SEMICOLON
    /* ProgramBody */
    DeclarationList FunctionList CompoundStatement
    /* End of ProgramBody */
    END {
        root = new ProgramNode(@1.first_line, @1.first_column, $1, $3, $4, $5);
        free($1);
    }
;

ProgramName:
    ID
;

DeclarationList:
    Epsilon {
        $$ = NULL;
    }
    |
    Declarations {
        $$ = $1;
    }
;

Declarations:
    Declaration{
        $$ = new std::vector<AstNode *>();
        $$->push_back($1);
    }
    |
    Declarations Declaration{
        $$ = $1;
        $$->push_back($2);
    }
;

FunctionList:
    Epsilon {
        $$ = NULL;
    }
    |
    Functions {
        $$ = $1;
    }
;

Functions:
    Function {
        $$ = new std::vector<AstNode *>();
        $$->push_back($1);
    }
    |
    Functions Function {
        $$ = $1;
        $$->push_back($2);
    }
;

Function:
    FunctionDeclaration {
        $$ = $1;
    }
    |
    FunctionDefinition {
        $$ = $1;
    }
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON {
        if ($3 != NULL){
            std::vector<DeclNode *> *decl_list = new std::vector<DeclNode *>();
            for (auto arg : *$3) {
                decl_list->push_back((DeclNode *)arg);
            }
            $$ = new FunctionNode(@1.first_line, @1.first_column, $1, decl_list, $5, NULL);
        }
        else{
            $$ = new FunctionNode(@1.first_line, @1.first_column, $1, nullptr, $5, NULL);
        }

    }
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType
    CompoundStatement
    END {
        if ($3 != NULL){
            std::vector<DeclNode *> *decl_list = new std::vector<DeclNode *>();
            for (auto arg : *$3) {
                decl_list->push_back((DeclNode *)arg);
            }
            $$ = new FunctionNode(@1.first_line, @1.first_column, $1, decl_list, $5, $6);
        }
        else{
            $$ = new FunctionNode(@1.first_line, @1.first_column, $1, nullptr, $5, $6);
        }

    }
;

FunctionName:
    ID
;

FormalArgList:
    Epsilon {
        $$ = NULL;
    }
    |
    FormalArgs {
        $$ = $1;
    }
;

FormalArgs:
    FormalArg {
        $$ = new std::vector<AstNode *>();
        $$->push_back($1);
    }
    |
    FormalArgs SEMICOLON FormalArg {
        $$ = $1;
        $$->push_back($3);
    }
;

FormalArg:
    IdList COLON Type {
        std::vector<VariableNode *> *var_list = new std::vector<VariableNode *>();

        for (auto id : *$1) {
            var_list->push_back(new VariableNode(id->line, id->column, id->val.c_str(), $3, NULL));
        }

        $$ = new DeclNode(@1.first_line, @1.first_column, var_list);
    }
;

IdList:
    ID {
        $$ = new std::vector<Variable *>();
        $$->push_back(new Variable($1, @1.first_line, @1.first_column));
    }
    |
    IdList COMMA ID {
        $$ = $1;
        $$->push_back(new Variable($3, @3.first_line, @3.first_column));
    }
;

ReturnType:
    COLON ScalarType {
        $$ = (char *)$2;
    }
    |
    Epsilon {
        $$ = (char *)"void";
    }
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON Type SEMICOLON {
        std::vector<VariableNode *> *var_list = new std::vector<VariableNode *>();
        
        for (auto id : *$2) {
            var_list->push_back(new VariableNode(id->line, id->column, id->val.c_str(), $4, NULL));
        }

        $$ = new DeclNode(@1.first_line, @1.first_column, var_list);
    }
    |
    VAR IdList COLON LiteralConstant SEMICOLON {
        std::vector<VariableNode *> *var_list = new std::vector<VariableNode *>();
        ConstantValueNode *const_val = new ConstantValueNode(@1.first_line, $4->column, $4->val.c_str());
        
        for (auto id : *$2){
            var_list->push_back(new VariableNode(id->line, id->column, id->val.c_str(), $4->type.c_str(), const_val));
        }

        $$ = new DeclNode(@1.first_line, @1.first_column, var_list);
    }
;

Type:
    ScalarType {
        $$ = $1;
    }
    |
    ArrType {
        $$ = $1;
    }
;

ScalarType:
    INTEGER {
        $$ = (char *)"integer";
    }
    |
    REAL {
        $$ = (char *)"real";
    }
    |
    STRING {
        $$ = (char *)"string";
    }
    |
    BOOLEAN {
        $$ = (char *)"boolean";
    }
;

ArrType:
    ArrDecl ScalarType {
        $$ = (char *)(std::string($2) + " " + std::string($1)).c_str();
    }
;

ArrDecl:
    ARRAY INT_LITERAL OF {
        $$ = (char *)("[" + std::to_string($2) + "]").c_str();
    }
    |
    ArrDecl ARRAY INT_LITERAL OF {
        $$ = (char *)(std::string($1) + "[" + std::to_string($3) + "]").c_str();
    }
;

LiteralConstant:
    NegOrNot INT_LITERAL {
        int num = $2 * $1;
        $$ = new Variable("integer", std::to_string(num));
        $$->column = ($1 == 1)?(@2.first_column):(@1.first_column);
    }
    |
    NegOrNot REAL_LITERAL {
        double num = $2 * $1;
        $$ = new Variable("real", std::to_string(num));
        $$->column = ($1 == 1)?(@2.first_column):(@1.first_column);
    }
    |
    StringAndBoolean {
        $$ = $1;
    }
;

NegOrNot:
    Epsilon {
        $$ = 1;
    }
    |
    MINUS %prec UNARY_MINUS {
        $$ = -1;
    }
;

StringAndBoolean:
    STRING_LITERAL {
        $$ = new Variable("string", $1);
        $$->column = @1.first_column;
    }
    |
    TRUE {
        $$ = new Variable("boolean", "true");
        $$->column = @1.first_column;
    }
    |
    FALSE {
        $$ = new Variable("boolean", "false");
        $$->column = @1.first_column;
    }
;

IntegerAndReal:
    INT_LITERAL {
        $$ = (char *)(std::to_string($1)).c_str();
    }
    |
    REAL_LITERAL {
        $$ = (char *)(std::to_string($1)).c_str();
    }
;

    /*
       Statements
                  */

Statement:
    CompoundStatement {
        $$ = $1;
    }
    |
    Simple {
        $$ = $1;
    }
    |
    Condition {
        $$ = $1;
    }
    |
    While {
        $$ = $1;
    }
    |
    For {
        $$ = $1;
    }
    |
    Return {
        $$ = $1;
    }
    |
    FunctionCall {
        $$ = $1;
    }
;

CompoundStatement:
    BEGIN_
    DeclarationList
    StatementList
    END {
        $$ = new CompoundStatementNode(@1.first_line, @1.first_column, $2, $3);
    }
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON {
        $$ = new AssignmentNode(@2.first_line, @2.first_column, $1, $3);
    }
    |
    PRINT Expression SEMICOLON {
        $$ = new PrintNode(@1.first_line, @1.first_column, $2);
    }
    |
    READ VariableReference SEMICOLON {
        $$ = new ReadNode(@1.first_line, @1.first_column, $2);
    }
;

VariableReference:
    ID ArrRefList {
        $$ = new VariableReferenceNode(@1.first_line, @1.first_column, $1, $2);
    }
;

ArrRefList:
    Epsilon {
        $$ = NULL;
    }
    |
    ArrRefs {
        $$ = $1;
    }
;

ArrRefs:
    L_BRACKET Expression R_BRACKET {
        $$ = new std::vector<AstNode *>();
        $$->push_back($2);
    }
    |
    ArrRefs L_BRACKET Expression R_BRACKET {
        $$ = $1;
        $$->push_back($3);
    }
;

Condition:
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF {
        $$ = new IfNode(@1.first_line, @1.first_column, $2, $4, $5);
    }
;

ElseOrNot:
    ELSE
    CompoundStatement {
        $$ = $2;
    }
    |
    Epsilon {
        $$ = NULL;
    }
;

While:
    WHILE Expression DO
    CompoundStatement
    END DO {
        $$ = new WhileNode(@1.first_line, @1.first_column, $2, $4);
    }
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO {
        DeclNode *decl = new DeclNode(@1.first_line, @2.first_column, 
                                        new std::vector<VariableNode *>({new VariableNode(@2.first_line, @2.first_column, $2, "integer", NULL)}));
        AssignmentNode *assign = new AssignmentNode(@1.first_line, @3.first_column, new VariableReferenceNode(@1.first_line, @2.first_column, $2, NULL), 
                                                        new ConstantValueNode(@1.first_line, @4.first_column, std::to_string($4).c_str()));
        ConstantValueNode *const_val = new ConstantValueNode(@1.first_line, @6.first_column, std::to_string($6).c_str());
        
        $$ = new ForNode(@1.first_line, @1.first_column, decl, assign, const_val, $8);
    }
;

Return:
    RETURN Expression SEMICOLON {
        $$ = new ReturnNode(@1.first_line, @1.first_column, $2);
    }
;

FunctionCall:
    FunctionInvocation SEMICOLON {
        $$ = $1;
    }
;

FunctionInvocation:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS {
        $$ = new FunctionInvocationNode(@1.first_line, @1.first_column, $1, $3);
    }
;

ExpressionList:
    Epsilon {
        $$ = NULL;
    }
    |
    Expressions {
        $$ = $1;
    }
;

Expressions:
    Expression {
        $$ = new std::vector<AstNode *>();
        $$->push_back($1);
    }
    |
    Expressions COMMA Expression {
        $$ = $1;
        $$->push_back($3);
    }
;

StatementList:
    Epsilon {
        $$ = NULL;
    }
    |
    Statements {
        $$ = $1;
    }
;

Statements:
    Statement {
        $$ = new std::vector<AstNode *>();
        $$->push_back($1);
    }
    |
    Statements Statement {
        $$ = $1;
        $$->push_back($2);
    }
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS {
        $$ = $2;
    }
    |
    MINUS Expression %prec UNARY_MINUS {
        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, "neg", $2);
    }
    |
    Expression MULTIPLY Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "*", $1, $3);
    }
    |
    Expression DIVIDE Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "/", $1, $3);
    }
    |
    Expression MOD Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "mod", $1, $3);
    }
    |
    Expression PLUS Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "+", $1, $3);
    }
    |
    Expression MINUS Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "-", $1, $3);
    }
    |
    Expression LESS Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<", $1, $3);
    }
    |
    Expression LESS_OR_EQUAL Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<=", $1, $3);
    }
    |
    Expression GREATER Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, ">", $1, $3);
    }
    |
    Expression GREATER_OR_EQUAL Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, ">=", $1, $3);
    }
    |
    Expression EQUAL Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "=", $1, $3);
    }
    |
    Expression NOT_EQUAL Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "<>", $1, $3);
    }
    |
    NOT Expression {
        $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, "not", $2);
    }
    |
    Expression AND Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "and", $1, $3);
    }
    |
    Expression OR Expression {
        $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, "or", $1, $3);
    }
    |
    IntegerAndReal {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1);
    }
    |
    StringAndBoolean {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column, $1->val.c_str());
    }
    |
    VariableReference {
        $$ = $1;
    }
    |
    FunctionInvocation {
        $$ = $1;
    }
;

    /*
       misc
            */
Epsilon:
;

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, current_line, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [--dump-ast]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        AstDumper ast_dumper;
        root->accept(ast_dumper);
    }

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}
