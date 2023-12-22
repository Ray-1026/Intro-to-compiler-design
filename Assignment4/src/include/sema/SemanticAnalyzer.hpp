#ifndef SEMA_SEMANTIC_ANALYZER_H
#define SEMA_SEMANTIC_ANALYZER_H

#include "AST/PType.hpp"
#include "visitor/AstNodeVisitor.hpp"
#include <iostream>
#include <vector>

#define FORMAT "%-33s%-11s%-11s%-17s%-11s\n"
#define demarcation_equal                                                                                              \
    {                                                                                                                  \
        puts(std::string(110, '=').c_str());                                                                           \
    }
#define demarcation_hyphen                                                                                             \
    {                                                                                                                  \
        puts(std::string(110, '-').c_str());                                                                           \
    }

extern uint32_t opt_dump;

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

class SymbolTable {
  public:
    void pushSymbol(std::string name, std::string kind, int level, std::string type, std::string attribute);
    void popSymbol() { entries.pop_back(); }
    void dumpSymbol();

    void addError(std::string error) { error_decls.emplace_back(error); }
    bool findError(std::string error);

    int getEntrySize() { return entries.size(); }
    std::string getLastEntryKind() { return entries[entries.size() - 1]->name; }
    std::string getLastEntryType() { return entries[entries.size() - 1]->type; }
    std::vector<SymbolEntry *> getEntry() { return entries; }

    SymbolEntry *findEntry(std::string name);
    bool checkRedeclaration(std::string name);
    void resetLastEntryKind(std::string kind)
    {
        if (!entries.empty())
            entries[entries.size() - 1]->kind = kind;
    }
    void resetLastEntryAttr(std::string attr)
    {
        if (!entries.empty())
            entries[entries.size() - 1]->attribute = attr;
    }

    std::string ret_name = "";

  private:
    std::vector<SymbolEntry *> entries;
    std::vector<std::string> error_decls;
};

class SymbolManager {
  public:
    void pushScope(SymbolTable *new_scope) { tables.emplace_back(new_scope); }
    void popScope() { tables.pop_back(); }
    void addLoopVar(std::string var) { loop_vars.emplace_back(var); }
    void popLoopVar() { loop_vars.pop_back(); }
    void addLoopIterCount(std::string count) { loop_iter_count.emplace_back(count); }
    SymbolTable *getTopScope() { return tables.back(); }
    SymbolTable *getPrevScope() { return (tables.size() > 1) ? tables[tables.size() - 2] : nullptr; }
    SymbolTable *getGlobalScope() { return tables[0]; }
    std::vector<SymbolTable *> getTables() { return tables; }

    bool checkLoopVar(std::string var);
    bool checkLoopIterCount();

  private:
    std::vector<SymbolTable *> tables;
    std::vector<std::string> loop_vars;
    std::vector<std::string> loop_iter_count;
};

class SemanticAnalyzer final : public AstNodeVisitor {
  private:
    // TODO: something like symbol manager (manage symbol tables)
    //       context manager, return type manager
    char **src;
    int lv = 0, encounter_bin = 0;
    bool encounter_func = false, encounter_for = false, encounter_varref = false, encounter_func_invo = false,
         encounter_assign = false, encounter_return = false, error_found = false;

    SymbolManager symbol_manager;

  public:
    std::vector<std::string> current_ret_type;

    ~SemanticAnalyzer() = default;
    SemanticAnalyzer(char **src) : src(src) {}

    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;

    bool hasError() { return error_found; }
};

#endif
