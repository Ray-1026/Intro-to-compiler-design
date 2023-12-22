# hw4 report

|||
|-:|:-|
|Name|蔡師睿|
|ID|110550093|

## How much time did you spend on this project

> About 30 hours QQ

## Project overview

### scanner.I
To record each line of source code, I define a dynamic array of pointer and a parameter to record its current size.
```c
char **src = NULL;
uint32_t src_size = 0;
```
Then, I use `strdup()` to record each line, and due to the dynamic array, I use `realloc()` when it is full.
```c
    if (line_num >= src_size) {
        src_size = line_num * 2;
        src = (char **)realloc(src, sizeof(char *) * (src_size));
    }
    src[line_num] = strdup(current_line);

    ++line_num;
```
Finally, I can pass the entire source code to *SemanticAnalyzer.hpp*.


Besides, to add the switch button of pseudocomment D, I define a variable `opt_dump` for turning on/off the symbol table dumping.

### parser.y
There is a variable called `error_found` to record the presence of any error codes, and a function `hasError()` to retrieve the boolean value of `error_found`. After semantic analysis, we can determine whether to display no syntactic error and semantic error message.

### AST and Semantic Analyzer
To construct symbol table, I use the structure provided on README.md, and add other functions or variables I need. Moreover, I define a `PtypeSharedPty ptype` in my `SymbolEntry` so that I can easily get its dimension or type in every visit. 
```cpp
class SymbolEntry {
  private:
  public:
    std::string name, kind, level, type, attribute;
    PTypeSharedPtr ptype;
    SymbolEntry(std::string name, std::string kind, int level, std::string type, std::string attribute)
        : name(name), kind(kind), type(type), attribute(attribute)
    {
        this->level = std::to_string(level) + ((level) ? ("(local)") : ("(global)"));

        std::string temp = type.substr(0, type.find(" "));
        if (temp == "void")
            ptype = std::make_shared<PType>(PType::PrimitiveTypeEnum::kVoidType);
        else if (temp == "integer")
            ptype = std::make_shared<PType>(PType::PrimitiveTypeEnum::kIntegerType);
        else if (temp == "real")
            ptype = std::make_shared<PType>(PType::PrimitiveTypeEnum::kRealType);
        else if (temp == "boolean")
            ptype = std::make_shared<PType>(PType::PrimitiveTypeEnum::kBoolType);
        else if (temp == "string")
            ptype = std::make_shared<PType>(PType::PrimitiveTypeEnum::kStringType);

        std::vector<uint64_t> dim;
        for (size_t i = 0; i < type.length(); i++) {
            if (type[i] == '[') {
                int j = i + 1;
                while (type[j] != ']')
                    j++;
                dim.emplace_back(std::stoi(type.substr(i + 1, j - i - 1)));
                i = j;
            }
        }
        ptype->setDimensions(dim);
    }
};
```
Then we can do semantic analyze via visting the AST to dump the tables and error messages.

## What is the hardest you think in this project

The entire assignment is quite challenging, and in my opinion, dealing with the error log is even more difficult than constructing the symbol table.

## Feedback to T.A.s

I think this is much harder than assignment 3. 
