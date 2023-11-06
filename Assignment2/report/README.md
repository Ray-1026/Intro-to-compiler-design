# hw2 report

|||
|-:|:-|
|Name|蔡師睿|
|ID|110550093|

## How much time did you spend on this project

> About 6 hours

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way. \
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here. 

### Tokens
Below is the token table.

|  Delimiter  |     Token       |   Operator   |    Token  |   Operator   |    Token  |
|:-----------:|:---------------:|:------------:|:---------:|:------------:|:---------:|
|   ","       |       ','       |     "+"      |     '+'   |      "<="    |     LE    |
|   ";"       |       ';'       |     "-"      |     '-'   |      "<>"    |     NE    |
|   ":"       |      ':'        |     "*"      |     '*'   |      ">"     |     '>'   |
|  "("        |       '('       |       "/"    |     '/'   |      ">="    |     GE    |
|  ")"        |       ')'       |     "mod"    |     MOD   |      "="     |     '='   |
|  "["        |       '['       |      ":="    |   ASSIGN  |      "and"   |     AND   |
|  "]"        |       ']'       |      "<"     |     '<'   |      "or"    |     OR    |
|             |                 |              |           |      "not"   |     NOT   |

| Reserved Word |     Token       | Reserved Word |     Token       |
|:-------------:|:---------------:|:-------------:|:---------------:|
| "var"         | KWvar           | "begin"       | KWbegin         |
| "array"       | KWarray         | "end"         | KWend           |
| "of"          | KWof            | "while"       | KWwhile         |
| "boolean"     | KWboolean       | "do"          | KWdo            |
| "integer"     | KWinteger       | "if"          | KWif            |
| "real"        | KWreal          | "then"        | KWthen          |
| "string"      | KWstring        | "else"        | KWelse          |
| "true"        | KWtrue          | "for"         | KWfor           |
| "false"       | KWfalse         | "to"          | KWto            |
| "def"         | KWdef           | "print"       | KWprint         |
| "return"      | KWreturn        | "read"        | KWread          |

| Others                       | Token   |
|:----------------------------:|:-------:|
| identifier                   | ID      |
| string                       | Str     |
| integer                      | Int     |
| 0[0-7]+                      | Oct_int |
| float                        | Float   |
| scientific notation          | Sci     |

### Define Associativities
The following is the associativities I defined in *parser.y*. From the top to the bottom, the associativity is from low to high precedence.

```
%left AND OR NOT
%left '<' '>' '=' LE GE NE
%left '-'
%left '+'
%left '/' MOD
%left '*'
```

### Grammar Rules
#### Program
As spec

#### Function
To make the argument list easier to parse, I defined the `id_list` and `args` as follows.
```
id_list: ID | id_list ',' ID;
args: | id_list ':' scalar_type;
```
And `scalar_type` is defined as follows.
```
scalar_type: KWinteger | KWreal | KWstring | KWboolean;
```
Others are as spec.

#### Variable Declaration
Define `array` to represent the structure of array.
```
array: KWarray const_int KWof scalar_type | KWarray const_int KWof array;
```
And `const_int` is defined as follows.
```
const_int: Int | Oct_int;
```
Others are as spec.

#### Constant Declaration
Define `const_literal` to represent the proper type.
```
const_literal: const_int | Float | Sci | Str | KWtrue | KWfalse;
```
`const_decl` is the constant declaration, and it should be considered the negative sign.
```
const_decl: KWvar id_list ':' const_literal ';'
            | KWvar id_list ':' '-' Int ';'
            | KWvar id_list ':' '-' Float ';'
            ;
```

#### Statement
Define `array_ref` to represent valid array reference (e.g. arr[30] and arr[30][30]). And define `var_ref` to represent valid variable reference.
```
array_ref: ID '[' expr ']' | array_ref '[' expr ']';
var_ref: ID | array_ref
```
The `expr` in the `array_ref` is Expression and will be introduced later. \
Others are as spec.

#### Expression
Use `%prec` if there is the unary operator.

#### Others
Define to represent the zero or more of the same type.
```
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
```

## What is the hardest you think in this project

I think the hardest part of this project is to make the proper grammar rules to parse the tokens. Besides, it's also important to read the spec carefully and make sure you thoroughly know how the P language works.

## Feedback to T.A.s

I learned a lot from this project. Thank you for your hard work.
