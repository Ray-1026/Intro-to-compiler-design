#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"

void SymbolTable::pushSymbol(std::string name, std::string kind, int level, std::string type, std::string attribute)
{
    SymbolEntry *entry = new SymbolEntry(name, kind, level, type, attribute);
    entries.emplace_back(entry);
}

bool SymbolTable::checkRedeclaration(std::string name)
{
    for (auto &entry : entries)
        if (entry->name == name)
            return true;
    return false;
}

void SymbolTable::dumpSymbol()
{
    if (!opt_dump)
        return;

    demarcation_equal;
    printf(FORMAT, "Name", "Kind", "Level", "Type", "Attribute");
    demarcation_hyphen;

    for (auto &entry : entries)
        printf(FORMAT, entry->name.c_str(), entry->kind.c_str(), entry->level.c_str(), entry->type.c_str(),
               entry->attribute.c_str());

    demarcation_hyphen;
}

bool SymbolTable::findError(std::string error)
{
    for (auto &err : error_decls)
        if (err == error)
            return true;
    return false;
}

SymbolEntry *SymbolTable::findEntry(std::string name)
{
    for (auto &entry : entries) {
        if (entry->name == name)
            return entry;
    }
    return nullptr;
}

bool SymbolManager::checkLoopVar(std::string var)
{
    for (auto &loop_var : loop_vars)
        if (loop_var == var)
            return true;
    return false;
}

bool SymbolManager::checkLoopIterCount()
{
    int x = atoi(loop_iter_count[0].c_str()), y = atoi(loop_iter_count[1].c_str());
    loop_iter_count.clear();
    return x <= y;
}

void SemanticAnalyzer::visit(ProgramNode &p_program)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // 1.
    SymbolTable *current_table = new SymbolTable();
    symbol_manager.pushScope(current_table);

    // 2.
    current_table->pushSymbol(p_program.getNameCString(), "program", lv, "void", "");
    current_ret_type.emplace_back("void");

    // 3.
    p_program.visitChildNodes(*this);

    // 4.
    current_table = symbol_manager.getTopScope();
    current_table->dumpSymbol();

    // 5.
    symbol_manager.popScope();
    current_ret_type.pop_back();
}

void SemanticAnalyzer::visit(DeclNode &p_decl) { p_decl.visitChildNodes(*this); }

void SemanticAnalyzer::visit(VariableNode &p_variable)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // 1.
    SymbolTable *current_table = symbol_manager.getTopScope();

    // 2.
    std::string name = p_variable.getNameCString();
    if (current_table->checkRedeclaration(name) || symbol_manager.checkLoopVar(name)) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg, "<Error> Found in line %%d, column %%d: symbol '%%s' is redeclared\n    %%s\n    %%%ds\n",
                p_variable.getLocation().col);
        fprintf(stderr, error_msg, p_variable.getLocation().line, p_variable.getLocation().col, name.c_str(),
                src[p_variable.getLocation().line], "^");

        current_table->addError(name);
    }
    else {
        std::string kind = (encounter_func) ? ("parameter") : (encounter_for) ? ("loop_var") : ("variable");
        current_table->pushSymbol(name, kind, lv, p_variable.getTypeCString(), "");
    }

    if (encounter_for)
        symbol_manager.addLoopVar(name);

    // 3.
    p_variable.visitChildNodes(*this);

    // 4.
    if (!p_variable.checkDimension()) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: '%%s' declared as an array with an index that is not greater "
                "than 0\n    %%s\n    %%%ds\n",
                p_variable.getLocation().col);
        fprintf(stderr, error_msg, p_variable.getLocation().line, p_variable.getLocation().col, name.c_str(),
                src[p_variable.getLocation().line], "^");
        current_table->addError(name);
    }
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    if (encounter_for) {
        symbol_manager.addLoopIterCount(p_constant_value.getConstantValueCString());
        return;
    }
    else if (encounter_varref || encounter_bin || encounter_func_invo || encounter_assign)
        return;

    symbol_manager.getTopScope()->resetLastEntryKind("constant");
    symbol_manager.getTopScope()->resetLastEntryAttr(p_constant_value.getConstantValueCString());
    p_constant_value.setType(p_constant_value.getConstantValueCString());
}

void SemanticAnalyzer::visit(FunctionNode &p_function)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // 1.
    SymbolTable *current_table = symbol_manager.getTopScope();
    symbol_manager.pushScope(new SymbolTable());
    encounter_func = true;

    int flag = 0;

    // 2.
    if (current_table->checkRedeclaration(p_function.getNameCString())) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg, "<Error> Found in line %%d, column %%d: symbol '%%s' is redeclared\n    %%s\n    %%%ds\n",
                p_function.getLocation().col);
        fprintf(stderr, error_msg, p_function.getLocation().line, p_function.getLocation().col,
                p_function.getNameCString(), src[p_function.getLocation().line], "^");
        current_table->addError(p_function.getNameCString());
    }
    else {
        std::string prototype = p_function.getPrototypeCString();
        int pos = prototype.find(" ");
        std::string type = prototype.substr(0, pos), attribute = prototype.substr(pos + 2);
        attribute = attribute.substr(0, attribute.length() - 1);

        current_table->pushSymbol(p_function.getNameCString(), "function", lv, type, attribute);
        current_ret_type.emplace_back(type);
        flag = 1;
    }

    // 3.
    lv++;
    p_function.visitChildNodes(*this);
    lv--;

    // 4.
    current_table = symbol_manager.getTopScope();
    current_table->dumpSymbol();

    // 5.
    symbol_manager.popScope();
    encounter_func = false;
    if (flag)
        current_ret_type.pop_back();
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // 1. 2.
    int flag = 0;
    encounter_for = false;
    if (!encounter_func) {
        symbol_manager.pushScope(new SymbolTable());
        flag = 1;
        lv++;
    }
    else
        encounter_func = false;

    // 3.
    p_compound_statement.visitChildNodes(*this);

    // 5.
    if (flag) {
        lv--;
        SymbolTable *current_table = symbol_manager.getTopScope();
        current_table->dumpSymbol();
        symbol_manager.popScope();
    }
}

void SemanticAnalyzer::visit(PrintNode &p_print)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_print.visitChildNodes(*this);

    // semantic analyses
    if (!p_print.checkChlid())
        return;

    if (p_print.getDimension() > 0 || p_print.getTargetCString() == "void") {
        error_found = true;
        char error_msg[1024];
        sprintf(
            error_msg,
            "<Error> Found in line %%d, column %%d: expression of print statement must be scalar type\n    %%s\n    "
            "%%%ds\n",
            p_print.getTargetLocationCol());
        fprintf(stderr, error_msg, p_print.getTargetLocationLine(), p_print.getTargetLocationCol(),
                src[p_print.getTargetLocationLine()], "^");
    }
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    encounter_bin++;
    p_bin_op.visitChildNodes(*this);
    encounter_bin--;

    std::string left_op = p_bin_op.getLeftOperandCString(), operand = p_bin_op.getOpCString(),
                right_op = p_bin_op.getRightOperandCString();

    if (left_op == "null" || right_op == "null")
        return;

    // error message format
    char error_msg[1024];
    sprintf(error_msg,
            "<Error> Found in line %%d, column %%d: invalid operands to binary operator '%s' ('%s' and "
            "'%s')\n    %%s\n    %%%ds\n",
            operand.c_str(), left_op.c_str(), right_op.c_str(), p_bin_op.getLocation().col);

    // semantic analyses
    if (operand == "+") {
        if ((left_op != right_op) &&
            ((left_op != "integer" && left_op != "real") || (right_op != "integer" && right_op != "real"))) {
            error_found = true;
            fprintf(stderr, error_msg, p_bin_op.getLocation().line, p_bin_op.getLocation().col,
                    src[p_bin_op.getLocation().line], "^");
            return;
        }

        if (left_op == "string" || right_op == "string")
            p_bin_op.setType("string");
        else
            p_bin_op.setType(left_op);
    }
    else if (operand == "-" || operand == "*" || operand == "/") {
        if ((left_op != "integer" && left_op != "real") || (right_op != "integer" && right_op != "real")) {
            error_found = true;
            fprintf(stderr, error_msg, p_bin_op.getLocation().line, p_bin_op.getLocation().col,
                    src[p_bin_op.getLocation().line], "^");
            return;
        }
        p_bin_op.setType(left_op);
    }
    else if (operand == "mod") {
        if (left_op != "integer" || right_op != "integer") {
            error_found = true;
            fprintf(stderr, error_msg, p_bin_op.getLocation().line, p_bin_op.getLocation().col,
                    src[p_bin_op.getLocation().line], "^");
            return;
        }
        p_bin_op.setType("integer");
    }
    else if (operand == "<" || operand == "<=" || operand == ">" || operand == ">=" || operand == "=" ||
             operand == "<>") {
        if ((left_op != "integer" && left_op != "real") || (right_op != "integer" && right_op != "real")) {
            error_found = true;
            fprintf(stderr, error_msg, p_bin_op.getLocation().line, p_bin_op.getLocation().col,
                    src[p_bin_op.getLocation().line], "^");
            return;
        }
        p_bin_op.setType("boolean");
    }
    else if (operand == "and" || operand == "or") {
        if (left_op != "boolean" || right_op != "boolean") {
            error_found = true;
            fprintf(stderr, error_msg, p_bin_op.getLocation().line, p_bin_op.getLocation().col,
                    src[p_bin_op.getLocation().line], "^");
            return;
        }
        p_bin_op.setType("boolean");
    }
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_un_op.visitChildNodes(*this);

    std::string operand = p_un_op.getOpCString(), op = p_un_op.getOperandCString();

    if (op == "null")
        return;

    char error_msg[1024];
    sprintf(error_msg,
            "<Error> Found in line %%d, column %%d: invalid operand to unary operator '%s' ('%s')\n    %%s\n    "
            "%%%ds\n",
            operand.c_str(), op.c_str(), p_un_op.getLocation().col);

    if (operand == "not") {
        if (op != "boolean") {
            error_found = true;
            fprintf(stderr, error_msg, p_un_op.getLocation().line, p_un_op.getLocation().col,
                    src[p_un_op.getLocation().line], "^");
            return;
        }
        p_un_op.setType("boolean");
    }
    else if (operand == "-") {
        if (op != "integer" && op != "real") {
            error_found = true;
            fprintf(stderr, error_msg, p_un_op.getLocation().line, p_un_op.getLocation().col,
                    src[p_un_op.getLocation().line], "^");
            return;
        }
        p_un_op.setType(op);
    }
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    encounter_func_invo = true;
    p_func_invocation.visitChildNodes(*this);
    encounter_func_invo = false;

    // semantic analyses
    SymbolTable *current_table = symbol_manager.getTopScope(), *global_table = symbol_manager.getGlobalScope();
    const char *name = p_func_invocation.getNameCString();

    // call of non-function symbol
    if ((global_table->findEntry(name) && global_table->findEntry(name)->kind != "function") ||
        (current_table->findEntry(name) && current_table->findEntry(name)->kind != "function")) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: call of non-function symbol '%%s'\n    %%s\n    %%%ds\n",
                p_func_invocation.getLocation().col);
        fprintf(stderr, error_msg, p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, name,
                src[p_func_invocation.getLocation().line], "^");
        return;
    }
    // use undeclared
    else if (!global_table->checkRedeclaration(name) && !current_table->checkRedeclaration(name)) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: use of undeclared symbol '%%s'\n    %%s\n    %%%ds\n",
                p_func_invocation.getLocation().col);
        fprintf(stderr, error_msg, p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, name,
                src[p_func_invocation.getLocation().line], "^");
        return;
    }

    SymbolEntry *ent = current_table->findEntry(name);
    if (!ent)
        ent = global_table->findEntry(name);

    int dim = (ent->attribute == "") ? (0) : (1);
    std::vector<std::string> args(1, "");
    for (size_t i = 0; i < ent->attribute.length(); i++) {
        switch (ent->attribute[i]) {
        case ',':
            dim++;
            args.emplace_back("");
            break;
        case ' ':
            if (ent->attribute[i - 1] != ',')
                args[dim - 1] += ent->attribute[i];
            break;
        default:
            args[dim - 1] += ent->attribute[i];
            break;
        }
    }

    // too few/much args
    if (p_func_invocation.getCurrentDimension() != dim) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: too few/much arguments provided for function '%%s'\n    "
                "%%s\n    %%%ds\n",
                p_func_invocation.getLocation().col);
        fprintf(stderr, error_msg, p_func_invocation.getLocation().line, p_func_invocation.getLocation().col, name,
                src[p_func_invocation.getLocation().line], "^");
        return;
    }

    // incompatible type
    std::vector<std::unique_ptr<ExpressionNode>> &args_node = p_func_invocation.getArgs();
    for (size_t i = 0; i < args.size() && i < args_node.size(); i++) {
        if (strcmp(args[i].c_str(), args_node[i]->getPTypeCString()) != 0) {
            error_found = true;
            char error_msg[1024];
            sprintf(error_msg,
                    "<Error> Found in line %%d, column %%d: incompatible type passing '%%s' to parameter of type "
                    "'%%s'\n    %%s\n    %%%ds\n",
                    args_node[i]->getLocation().col);
            fprintf(stderr, error_msg, p_func_invocation.getLocation().line, args_node[i]->getLocation().col,
                    args_node[i]->getPTypeCString(), args[i].c_str(), src[p_func_invocation.getLocation().line], "^");
            return;
        }
    }

    p_func_invocation.setType(ent->type);
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    encounter_varref = true;
    p_variable_ref.visitChildNodes(*this);
    encounter_varref = false;

    // semantic analyses
    SymbolTable *current_table = symbol_manager.getTopScope(), *prev_table = symbol_manager.getPrevScope();
    const char *name = p_variable_ref.getNameCString();
    SymbolEntry *current_entry = current_table->findEntry(name), *prev_entry = prev_table->findEntry(name);
    char error_msg[1024];

    if (encounter_return)
        current_table->ret_name = name;

    // use function or program name as variable
    if (prev_entry && (prev_entry->kind == "function" || prev_entry->kind == "program")) {
        error_found = true;
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: use of non-variable symbol '%%s'\n    %%s\n    %%%ds\n",
                p_variable_ref.getLocation().col);
        fprintf(stderr, error_msg, p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, name,
                src[p_variable_ref.getLocation().line], "^");
        return;
    }
    // use undeclared variable
    else if (!encounter_for && (!current_entry && !prev_entry)) {
        error_found = true;
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: use of undeclared symbol '%%s'\n    %%s\n    %%%ds\n",
                p_variable_ref.getLocation().col);
        fprintf(stderr, error_msg, p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, name,
                src[p_variable_ref.getLocation().line], "^");
        return;
    }

    if (current_table->findError(name))
        return;

    ExpressionNode *err_expr = p_variable_ref.errorIndex();

    // over array subscript
    if (current_entry && p_variable_ref.getCurrentDimension() > current_entry->ptype->getDimension()) {
        error_found = true;
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: there is an over array subscript on '%%s'\n    %%s\n    "
                "%%%ds\n",
                p_variable_ref.getLocation().col);
        fprintf(stderr, error_msg, p_variable_ref.getLocation().line, p_variable_ref.getLocation().col, name,
                src[p_variable_ref.getLocation().line], "^");

        symbol_manager.getTopScope()->ret_name = "";
        return;
    }
    // index of array reference is not integer
    else if (err_expr && strcmp(err_expr->getPTypeCString(), "null")) {
        error_found = true;
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: index of array reference must be an integer\n    %%s\n    "
                "%%%ds\n",
                err_expr->getLocation().col);
        fprintf(stderr, error_msg, p_variable_ref.getLocation().line, err_expr->getLocation().col,
                src[p_variable_ref.getLocation().line], "^");
        return;
    }

    if (current_entry) {
        p_variable_ref.setType(current_entry->type.substr(0, current_entry->type.find(" ")));
        p_variable_ref.updateDimension(current_entry->type);
    }
    else {
        p_variable_ref.setType(prev_entry->type.substr(0, prev_entry->type.find(" ")));
        p_variable_ref.updateDimension(prev_entry->type);
    }
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    encounter_assign = true;
    p_assignment.visitChildNodes(*this);
    encounter_assign = false;

    // semantic analyses
    if (!strcmp(p_assignment.getLValueCString(), "null"))
        return;

    SymbolTable *current_table = symbol_manager.getTopScope(), *prev_table = symbol_manager.getPrevScope();
    const char *name = p_assignment.getLValue()->getNameCString(),
               *lvalue_type = p_assignment.getLValue()->getPTypeCString();
    char error_msg[1024];

    // variable reference be an array type
    if (p_assignment.getLValueDimension() > 0) {
        error_found = true;
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: array assignment is not allowed\n    %%s\n    %%%ds\n",
                p_assignment.getLValue()->getLocation().col);
        fprintf(stderr, error_msg, p_assignment.getLocation().line, p_assignment.getLValue()->getLocation().col,
                src[p_assignment.getLocation().line], "^");
        return;
    }
    // assign to constant
    else if (current_table->findEntry(name) && current_table->findEntry(name)->kind == "constant") {
        error_found = true;
        sprintf(
            error_msg,
            "<Error> Found in line %%d, column %%d: cannot assign to variable '%%s' which is a constant\n    %%s\n    "
            "%%%ds\n",
            p_assignment.getLValue()->getLocation().col);
        fprintf(stderr, error_msg, p_assignment.getLocation().line, p_assignment.getLValue()->getLocation().col, name,
                src[p_assignment.getLocation().line], "^");
        return;
    }
    // modify loop var
    else if (!encounter_for && prev_table->findEntry(name) && prev_table->findEntry(name)->kind == "loop_var") {
        error_found = true;
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: the value of loop variable cannot be modified inside the loop "
                "body\n    %%s\n    "
                "%%%ds\n",
                p_assignment.getLValue()->getLocation().col);
        fprintf(stderr, error_msg, p_assignment.getLocation().line, p_assignment.getLValue()->getLocation().col,
                src[p_assignment.getLocation().line], "^");
        return;
    }

    if (!strcmp(p_assignment.getExprCString(), "null"))
        return;

    const char *expr_type = p_assignment.getExpr()->getPTypeCString();

    // array assignment
    if (expr_type[strlen(expr_type) - 1] == ']') {
        error_found = true;
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: array assignment is not allowed\n    %%s\n    %%%ds\n",
                p_assignment.getExpr()->getLocation().col);
        fprintf(stderr, error_msg, p_assignment.getLocation().line, p_assignment.getExpr()->getLocation().col,
                src[p_assignment.getLocation().line], "^");
        return;
    }
    // incompatible type
    else if (strcmp(lvalue_type, expr_type) != 0 &&
             (strcmp(lvalue_type, "real") != 0 || strcmp(expr_type, "integer") != 0) &&
             (strcmp(lvalue_type, "integer") != 0 || strcmp(expr_type, "real") != 0)) {
        error_found = true;
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: assigning to '%%s' from incompatible type '%%s'\n    %%s\n    "
                "%%%ds\n",
                p_assignment.getLocation().col);
        fprintf(stderr, error_msg, p_assignment.getLocation().line, p_assignment.getLocation().col,
                p_assignment.getLValue()->getPTypeCString(), expr_type, src[p_assignment.getLocation().line], "^");
        return;
    }
}

void SemanticAnalyzer::visit(ReadNode &p_read)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_read.visitChildNodes(*this);

    // semantic analyses
    if (!p_read.checkChlid())
        return;

    // not scalar type
    if (p_read.getDimension() > 0) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: variable reference of read statement must be scalar type\n    "
                "%%s\n    "
                "%%%ds\n",
                p_read.getTargetLocationCol());
        fprintf(stderr, error_msg, p_read.getTargetLocationLine(), p_read.getTargetLocationCol(),
                src[p_read.getTargetLocationLine()], "^");
        return;
    }

    // read constant or loop variable
    for (auto &table : symbol_manager.getTables()) {
        SymbolEntry *tmp = table->findEntry(p_read.getNameCString());
        if (tmp && (tmp->kind == "constant" || tmp->kind == "loop_var")) {
            error_found = true;
            char error_msg[1024];
            sprintf(error_msg,
                    "<Error> Found in line %%d, column %%d: variable reference of read statement cannot be a "
                    "constant or loop variable\n    %%s\n    %%%ds\n",
                    p_read.getTargetLocationCol());
            fprintf(stderr, error_msg, p_read.getTargetLocationLine(), p_read.getTargetLocationCol(),
                    src[p_read.getTargetLocationLine()], "^");
            return;
        }
    }
}

void SemanticAnalyzer::visit(IfNode &p_if)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_if.visitChildNodes(*this);

    if (strcmp(p_if.getConditionType(), "null") == 0)
        return;

    // semantic analyses
    if (strcmp(p_if.getConditionType(), "boolean") != 0) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: the expression of condition must be boolean type\n    %%s\n    "
                "%%%ds\n",
                p_if.getCondition()->getLocation().col);
        fprintf(stderr, error_msg, p_if.getLocation().line, p_if.getCondition()->getLocation().col,
                src[p_if.getLocation().line], "^");
    }
}

void SemanticAnalyzer::visit(WhileNode &p_while)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    p_while.visitChildNodes(*this);

    if (strcmp(p_while.getConditionType(), "null") == 0)
        return;

    // semantic analyses
    if (strcmp(p_while.getConditionType(), "boolean") != 0) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: the expression of condition must be boolean type\n    %%s\n    "
                "%%%ds\n",
                p_while.getCondition()->getLocation().col);
        fprintf(stderr, error_msg, p_while.getLocation().line, p_while.getCondition()->getLocation().col,
                src[p_while.getLocation().line], "^");
    }
}

void SemanticAnalyzer::visit(ForNode &p_for)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // 1. 2.
    SymbolTable *current_table = symbol_manager.getTopScope();
    symbol_manager.pushScope(new SymbolTable());
    encounter_for = true;

    // 3.
    lv++;
    p_for.visitChildNodes(*this);
    lv--;

    // 4.
    if (!symbol_manager.checkLoopIterCount()) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: the lower bound and upper bound of iteration count must be in "
                "the incremental order\n    %%s\n    %%%ds\n",
                p_for.getLocation().col);
        fprintf(stderr, error_msg, p_for.getLocation().line, p_for.getLocation().col, src[p_for.getLocation().line],
                "^");
    }

    // 5.
    current_table = symbol_manager.getTopScope();
    current_table->dumpSymbol();
    symbol_manager.popScope();
    symbol_manager.popLoopVar();
}

void SemanticAnalyzer::visit(ReturnNode &p_return)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    encounter_return = true;
    p_return.visitChildNodes(*this);
    encounter_return = false;

    // semantic analyses
    SymbolTable *prev_table = symbol_manager.getPrevScope(), *current_table = symbol_manager.getTopScope();
    const char *ret_type = p_return.getRetValCString();

    if (strcmp("void", current_ret_type.back().c_str()) == 0) {
        error_found = true;
        char error_msg[1024];
        sprintf(
            error_msg,
            "<Error> Found in line %%d, column %%d: program/procedure should not return a value\n    %%s\n    %%%ds\n",
            p_return.getLocation().col);
        fprintf(stderr, error_msg, p_return.getLocation().line, p_return.getLocation().col,
                src[p_return.getLocation().line], "^");
        return;
    }

    for (auto &entry : current_table->getEntry()) {
        if (entry->name == current_table->ret_name && strcmp(ret_type, "null") == 0) {
            ret_type = entry->type.c_str();
            break;
        }
    }

    if (strcmp(ret_type, "null") == 0)
        return;

    if (strcmp(ret_type, prev_table->getLastEntryType().c_str()) != 0) {
        error_found = true;
        char error_msg[1024];
        sprintf(error_msg,
                "<Error> Found in line %%d, column %%d: return '%%s' from a function with return type "
                "'%%s'\n    %%s\n    %%%ds\n",
                p_return.getRetVal()->getLocation().col);
        fprintf(stderr, error_msg, p_return.getLocation().line, p_return.getRetVal()->getLocation().col, ret_type,
                prev_table->getLastEntryType().c_str(), src[p_return.getLocation().line], "^");
        return;
    }
}
