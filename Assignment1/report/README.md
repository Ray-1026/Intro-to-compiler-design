# hw1 report

|Field|Value|
|-:|:-|
|Name|蔡師睿|
|ID|110550093|

## How much time did you spend on this project

About 6-7 hours.

## Project overview

### Definitions Section
#### C declarations and includes (line 1~20)
Use the provided.

#### Regex (line 22~31)
Define the regular expressions for tokens.

#### Declare states (line 35)
`c_comment` denotes the condition of c-style comment.
`cpp_comment` denotes the condition of c++-style comment.


### Rules Section
#### Delimeters (line 37~43)
Use `LIST_TOKEN` macro to print out delimeters.

#### Arithmetic, relational, and logical operators (line 46~60)
Use `LIST_TOKEN` macro to print out arithmetic, relational, and logical operators.

#### Reserved words (line 63~90)
Use `LIST_TOKEN` macro to print out reserved words.

#### Identifiers (line 93)
Use `LIST_LITERAL` macro to print out both the token `id` and the string that constitutes the token.

#### Integer constants (line 96~97)
Use `LIST_LITERAL` macro to print out both the token `integer` or `oct_integer` and the string that constitutes the token.

#### Floating-point constants (line 100)
Use `LIST_LITERAL` macro to print out both the token `float` and the string that constitutes the token.

#### Scientific notations (line 103)
Use `LIST_LITERAL` macro to print out both the token `scientific` and the string that constitutes the token.

#### String constants (line 106~116)
Run for loop to detect whether there is any two consecutive double quotes; if yes, place two consecutive double quotes to one double quote. After for loop, use `LIST_LITERAL` macro to print out both the token `string` and the string that constitutes the token.

#### Whitespace (line 119)
Use `LIST_SOURCE`.

#### Newlines (line 120~136)
If the status is in `INITIAL` or `c_comment`, use the provided code.

If the status is in `cpp_comment`, also use the provided code but change the status to `INITIAL` in the end.

#### Comments (line 139~145)
If `/*` is detected, use `LIST_SOURCE,` change the status to `c_comment,` and then use `LIST_SOURCE` repeatedly until `*/` is deteced.

If `//` is detected, use `LIST_SOURCE,` change the status to `cpp_comment,` and then use `LIST_SOURCE` repeatedly until the status is no longer `cpp_comment`.

#### Pseudocomments (line 148~158)
Use `LIST_SOURCE`. If the option is `S`, turn `opt_src` on or off based on the following letter: `+` or `-`; and if the option is `T`, turn `opt_tok` on or off based on the following letter: `+` or `-`.

#### Error handling (line 161~164)
Use the provided.

### Routines Section (line 168~197)
Use the provided.

## What is the hardest you think in this project

Learn how to write lex, and I also consider the comments part to be one of the most challenging in this assignment because I have to think of all types of comments and do different operations accordingly while scanning through the **`P`** language.

## Feedback to T.A.s

The spec is written really clearly.