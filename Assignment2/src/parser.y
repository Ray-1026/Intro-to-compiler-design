%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t line_num;    /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

extern int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%token MOD LE NE GE ASSIGN AND OR NOT
%token KWvar KWarray KWof KWboolean KWinteger KWreal KWstring KWtrue KWfalse
%token KWdef KWreturn KWbegin KWend KWwhile KWdo KWif KWthen KWelse KWfor KWto KWprint KWread
%token ID Int Oct_int Float Sci Str

%left AND OR NOT
%left '<' '>' '=' LE GE NE
%left '-'
%left '+'
%left '/' MOD
%left '*'

%%
    /* program */
ProgramName: ID ';' zeroORmore_decl zeroORmore_func compound KWend

    /* formal argument */
id_list: ID | id_list ',' ID;
args: | id_list ':' scalar_type;

    /* function */
func_return_type: | ':' scalar_type;
func_decl: ID '(' args ')' func_return_type ';'
func_def: ID '(' args ')' func_return_type compound KWend

    /* data type and structure */
scalar_type: KWinteger | KWreal | KWstring | KWboolean;
array: KWarray const_int KWof scalar_type | KWarray const_int KWof array;
const_int: Int | Oct_int;
const_literal: const_int | Float | Sci | Str | KWtrue | KWfalse;

    /* variable */
var_decl: KWvar id_list ':' scalar_type ';'
        | KWvar id_list ':' array ';'
        ;

    /* constant */
const_decl: KWvar id_list ':' const_literal ';'
            | KWvar id_list ':' '-' Int ';'
            | KWvar id_list ':' '-' Float ';'
            ;

    /* compound */
compound: KWbegin zeroORmore_decl zeroORmore_state KWend
        | KWbegin zeroORmore_decl zeroORmore_state compound zeroORmore_decl zeroORmore_state KWend
        ;

    /* simple */
array_ref: ID '[' expr ']' | array_ref '[' expr ']';
var_ref: ID | array_ref
assignment: ID ASSIGN expr ';'
            | array_ref ASSIGN expr ';'
            | KWprint expr ';'
            | KWread var_ref ';'
            ;

    /* conditional */
else_cond: | KWelse compound
conditional: KWif expr KWthen compound else_cond KWend KWif

    /* loop */
while_: KWwhile expr KWdo compound KWend KWdo
for_: KWfor ID ASSIGN Int KWto Int KWdo compound KWend KWdo

    /* return */
ret: KWreturn expr ';'

    /* function call */
func_call: ID '(' zeroORmore_expr ')'

    /* expression */
expr: var_ref
    | const_literal
    | func_call
    | '-' expr %prec '*'
    | expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | expr MOD expr
    | '(' expr ')'
    | expr '<' expr
    | expr '>' expr
    | expr '=' expr
    | expr LE expr
    | expr GE expr
    | expr NE expr
    | expr AND expr
    | expr OR expr
    | NOT expr
    ;

    /* zero or more */
zeroORmore_func: | zeroORmore_func func_decl | zeroORmore_func func_def;
zeroORmore_decl: | zeroORmore_decl var_decl | zeroORmore_decl const_decl;
zeroORmore_state: 
                | zeroORmore_state assignment 
                | zeroORmore_state conditional
                | zeroORmore_state while_
                | zeroORmore_state for_
                | zeroORmore_state ret
                | zeroORmore_state func_call ';'
                ;
zeroORmore_expr: | expr | zeroORmore_expr ',' expr;

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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    fclose(yyin);
    yylex_destroy();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
