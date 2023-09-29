%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LIST_SOURCE                 appendToCurrentLine(yytext)
#define LIST_TOKEN(name)            do { LIST_SOURCE; if(opt_tok) printf("<%s>\n", name); } while(0)
#define LIST_LITERAL(name, literal) do { LIST_SOURCE; if(opt_tok) printf("<%s: %s>\n", name, literal); } while(0)
#define MAX_LINE_LENG               512

uint32_t line_num = 1;
static char current_line[MAX_LINE_LENG];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char *current_line_ptr = current_line;

static void appendToCurrentLine(const char *yytext_ptr);

%}

digit [0-9]
letter [a-zA-Z]
id {letter}({letter}|{digit})*
dec_integer 0|([1-9]{digit}*)
oct_integer 0[0-7]*
float (0|[1-9]{digit}*)\.(0|{digit}*[1-9])
scientific (([1-9]{digit}*)\.({digit}*)|([1-9]{digit}*))[eE][+-]?{dec_integer}
str \"([^\"]|\"{2})*\"
space [ \t]+
pseudocomment [/]{2}\&[A-Z][+-].*

%x c_comment cpp_comment

%%
    /*Delimeters*/
"," {LIST_TOKEN(",");}
";" {LIST_TOKEN(";");}
":" {LIST_TOKEN(":");}
"(" {LIST_TOKEN("(");}
")" {LIST_TOKEN(")");}
"[" {LIST_TOKEN("[");}
"]" {LIST_TOKEN("]");}

    /*Arithmetic, Relational, and Logical Operators*/
"+"   {LIST_TOKEN("+");}
"-"   {LIST_TOKEN("-");}
"*"   {LIST_TOKEN("*");}
"/"   {LIST_TOKEN("/");}
"mod" {LIST_TOKEN("mod");}
":="  {LIST_TOKEN(":=");}
"<"   {LIST_TOKEN("<");}
"<="  {LIST_TOKEN("<=");}
"<>"  {LIST_TOKEN("<>");}
">="  {LIST_TOKEN(">=");}
">"   {LIST_TOKEN(">");}
"="   {LIST_TOKEN("=");}
"and" {LIST_TOKEN("and");}
"or"  {LIST_TOKEN("or");}
"not" {LIST_TOKEN("not");}

    /*Reserved Words*/
    /*Reserved Words : declaration*/
"var"     {LIST_TOKEN("KWvar");}
"def"     {LIST_TOKEN("KWdef");}
    /*Reserved Words : type*/
"array"   {LIST_TOKEN("KWarray");}
"of"      {LIST_TOKEN("KWof");}
"boolean" {LIST_TOKEN("KWboolean");}
"integer" {LIST_TOKEN("KWinteger");}
"real"    {LIST_TOKEN("KWreal");}
"string"  {LIST_TOKEN("KWstring");}
    /*Reserved Words : value*/
"true"    {LIST_TOKEN("KWtrue");}
"false"   {LIST_TOKEN("KWfalse");}
    /*Reserved Words : flow-of-control*/
"while"   {LIST_TOKEN("KWwhile");}
"do"      {LIST_TOKEN("KWdo");}
"if"      {LIST_TOKEN("KWif");}
"then"    {LIST_TOKEN("KWthen");}
"else"    {LIST_TOKEN("KWelse");}
"for"     {LIST_TOKEN("KWfor");}
"to"      {LIST_TOKEN("KWto");}
    /*Reserved Words : block*/
"begin"   {LIST_TOKEN("KWbegin");}
"end"     {LIST_TOKEN("KWend");}
    /*Reserved Words : statement*/
"print"   {LIST_TOKEN("KWprint");}
"read"    {LIST_TOKEN("KWread");}
"return"  {LIST_TOKEN("KWreturn");}

    /*Identidiers*/
{id} {LIST_LITERAL("id", yytext);}

    /*Integer Constants*/
{dec_integer} {LIST_LITERAL("integer", yytext);}
{oct_integer} {LIST_LITERAL("oct_integer", yytext);}

    /*Floating-Point Constants*/
{float} {LIST_LITERAL("float", yytext);}

    /*Scientific Notations*/
{scientific} {LIST_LITERAL("scientific", yytext);}

    /*String Constants*/
{str} {
    char str_const[yyleng];
    int ptr=0;
    for(int i=1; i<yyleng-1; i++, ptr++){
        if(yytext[i] == '\"' && yytext[i+1] == '\"')
            i++;
        str_const[ptr]=yytext[i];
    }
    str_const[ptr]='\0';
    LIST_LITERAL("string", str_const);
}

    /*Whitespace*/
{space} {LIST_SOURCE;}
<INITIAL,c_comment>\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, current_line);
    }
    ++line_num;
    current_line[0] = '\0';
    current_line_ptr = current_line;
}
<cpp_comment>\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, current_line);
    }
    ++line_num;
    current_line[0] = '\0';
    current_line_ptr = current_line;
    BEGIN INITIAL;
}

    /*Comments*/
    /*Comments : c-style*/
"/*"            {LIST_SOURCE; BEGIN c_comment;}
<c_comment>.    {LIST_SOURCE;}
<c_comment>"*/" {LIST_SOURCE; BEGIN INITIAL;}
    /*Comments : c++-style*/
"//"           {LIST_SOURCE; BEGIN cpp_comment;}
<cpp_comment>. {LIST_SOURCE;}

    /*Pseudocomments*/
{pseudocomment} {
    LIST_SOURCE;
    switch(yytext[3]){
        case 'S':
            opt_src = (yytext[4]=='+')?(1):(0);
            break;
        case 'T':
            opt_tok = (yytext[4]=='+')?(1):(0);
            break;
    }
}

    /* Catch the character which is not accepted by rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%

static void appendToCurrentLine(const char *yytext_ptr) {
    while (*yytext_ptr) {
        *current_line_ptr = *yytext_ptr;
        ++current_line_ptr;
        ++yytext_ptr;
    }
    *current_line_ptr = '\0';
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    fclose(yyin);
    /* Free resources used by the scanner; otherwise, unused data structures
    will persist throughout the successive compiler phases. */
    yylex_destroy();

    return 0;
}
