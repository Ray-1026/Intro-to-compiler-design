#include "codegen/CodeGenerator.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>

CodeGenerator::CodeGenerator(const std::string &source_file_name, const std::string &save_path,
                             const SymbolManager *const p_symbol_manager)
    : m_symbol_manager_ptr(p_symbol_manager), m_source_file_path(source_file_name)
{
    // FIXME: assume that the source file is always xxxx.p
    const auto &real_path = save_path.empty() ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind("/");
    auto dot_pos = source_file_name.rfind(".");

    if (slash_pos != std::string::npos) {
        ++slash_pos;
    }
    else {
        slash_pos = 0;
    }
    auto output_file_path{real_path + "/" + source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S"};
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

void CodeGenerator::visit(ProgramNode &p_program)
{
    // Generate RISC-V instructions for program header
    // clang-format off
    constexpr const char *const riscv_assembly_file_prologue =
        "    .file \"%s\"\n"
        "    .option nopic\n";

    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_file_prologue, m_source_file_path.c_str());

    // Reconstruct the hash table for looking up the symbol entry
    // Hint: Use symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(p_program.getSymbolTable());

    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(*this); };
    for_each(p_program.getDeclNodes().begin(), p_program.getDeclNodes().end(), visit_ast_node);
    for_each(p_program.getFuncNodes().begin(), p_program.getFuncNodes().end(), visit_ast_node);

    constexpr const char *const format = ".section    .text\n"
                                         "    .align 2\n"
                                         "    .globl main\n"
                                         "    .type main, @function\nmain:\n";
    dumpInstructions(m_output_file.get(), format);

    const_cast<CompoundStatementNode &>(p_program.getBody()).accept(*this);

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_program.getSymbolTable());

    dumpInstructions(m_output_file.get(), "    .size main, .-main\n");

    // close file
    fclose(m_output_file.get());
}

void CodeGenerator::visit(DeclNode &p_decl) { p_decl.visitChildNodes(*this); }

void CodeGenerator::visit(VariableNode &p_variable)
{
    SymbolEntry *entry = m_symbol_manager_ptr->lookup(p_variable.getName());

    // global variables
    if (entry->getLevel() == 0 && entry->getKind() == SymbolEntry::KindEnum::kVariableKind) {
        dumpInstructions(m_output_file.get(), ".comm %s, 4, 4\n", p_variable.getNameCString());
    }
    // global constants
    else if (entry->getLevel() == 0 && entry->getKind() == SymbolEntry::KindEnum::kConstantKind) {
        dumpInstructions(m_output_file.get(),
                         ".section    .rodata\n    .align 2\n    .globl %s\n    .type %s, @object\n%s:\n    .word %s\n",
                         p_variable.getNameCString(), p_variable.getNameCString(), p_variable.getNameCString(),
                         entry->getAttribute().constant()->getConstantValueCString());
    }
    // local variables
    else if (entry->getLevel() != 0 && (entry->getKind() == SymbolEntry::KindEnum::kVariableKind ||
                                        entry->getKind() == SymbolEntry::KindEnum::kParameterKind ||
                                        entry->getKind() == SymbolEntry::KindEnum::kLoopVarKind)) {
        local_address_offset += 4;
        entry->setOffset(local_address_offset);

        if (entry->getKind() == SymbolEntry::KindEnum::kParameterKind)
            func_parameter++;
    }
    // local constants
    else if (entry->getLevel() != 0 && entry->getKind() == SymbolEntry::KindEnum::kConstantKind) {
        constexpr const char *const format_begin = "    addi t0, s0, -%d\n"
                                                   "    addi sp, sp, -4\n"
                                                   "    sw t0, 0(sp)\n";
        local_address_offset += 4;
        entry->setOffset(local_address_offset);
        dumpInstructions(m_output_file.get(), format_begin, local_address_offset);

        p_variable.visitChildNodes(*this);

        constexpr const char *const format_end = "    lw t0, 0(sp)\n"
                                                 "    addi sp, sp, 4\n"
                                                 "    lw t1, 0(sp)\n"
                                                 "    addi sp, sp, 4\n"
                                                 "    sw t0, 0(t1)\n";
        dumpInstructions(m_output_file.get(), format_end);
    }
}

void CodeGenerator::visit(ConstantValueNode &p_constant_value)
{
    constexpr const char *const format = "    li t0, %s\n"
                                         "    addi sp, sp, -4\n"
                                         "    sw t0, 0(sp)\n";
    dumpInstructions(m_output_file.get(), format, p_constant_value.getConstantValueCString());
}

void CodeGenerator::visit(FunctionNode &p_function)
{
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(p_function.getSymbolTable());

    const char *func_name = p_function.getNameCString();
    dumpInstructions(m_output_file.get(),
                     ".section    .text\n    .align 2\n    .globl %s\n    .type %s, @function\n%s:\n", func_name,
                     func_name, func_name);

    local_address_offset = 8;
    p_function.visitChildNodes(*this);

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_function.getSymbolTable());

    dumpInstructions(m_output_file.get(), "    .size %s, .-%s\n", func_name, func_name);
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement)
{
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(p_compound_statement.getSymbolTable());

    if (if_condition || while_condition) {
        constexpr const char *const format = "L%d:";
        dumpInstructions(m_output_file.get(), format, label++);
    }
    else if (for_condition) {
        constexpr const char *const format = "    lw t0, 0(sp)\n"
                                             "    addi sp, sp, 4\n"
                                             "    lw t1, 0(sp)\n"
                                             "    addi sp, sp, 4\n"
                                             "    bge t1, t0, L%d\n"
                                             "L%d:\n";
        dumpInstructions(m_output_file.get(), format, pseudo_label + 2, pseudo_label + 1);
        // ++label;
    }
    else {
        constexpr const char *const function_prologue = "    # in the function prologue\n"
                                                        "    addi sp, sp, -128\n"
                                                        "    sw ra, 124(sp)\n"
                                                        "    sw s0, 120(sp)\n"
                                                        "    addi s0, sp, 128\n\n";
        dumpInstructions(m_output_file.get(), function_prologue);
        local_address_offset = 8;

        // function parameters
        while (func_parameter) {
            local_address_offset += 4;
            constexpr const char *const format = "    sw a%d, -%d(s0)\n";
            dumpInstructions(m_output_file.get(), format, ((local_address_offset - 12) / 4) % 8, local_address_offset);
            func_parameter--;
        }
    }

    p_compound_statement.visitChildNodes(*this);

    if (if_condition) {
        if (else_condition) {
            constexpr const char *const format = "    j L%d\n";
            dumpInstructions(m_output_file.get(), format, label + 1);

            else_condition--;
        }
    }
    else if (while_condition) {
        constexpr const char *const format = "    j L%d\n";
        dumpInstructions(m_output_file.get(), format, label - 2);
    }
    else if (for_condition) {
    }
    else {
        constexpr const char *const function_epilogue = "\n    # in the function epilogue\n"
                                                        "    lw ra, 124(sp)\n"
                                                        "    lw s0, 120(sp)\n"
                                                        "    addi sp, sp, 128\n"
                                                        "    jr ra\n";
        dumpInstructions(m_output_file.get(), function_epilogue);
    }

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_compound_statement.getSymbolTable());
}

void CodeGenerator::visit(PrintNode &p_print)
{
    dumpInstructions(m_output_file.get(), "\n    # print\n");

    p_print.visitChildNodes(*this);

    constexpr const char *const format = "    lw a0, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    jal ra, printInt\n";
    dumpInstructions(m_output_file.get(), format);
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op)
{
    op++;
    p_bin_op.visitChildNodes(*this);
    op--;

    constexpr const char *const format = "    lw t0, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    lw t1, 0(sp)\n"
                                         "    addi sp, sp, 4\n";
    constexpr const char *const end_format = "    addi sp, sp, -4\n"
                                             "    sw t0, 0(sp)\n";

    dumpInstructions(m_output_file.get(), format);
    if (strcmp(p_bin_op.getOpCString(), "+") == 0) {
        dumpInstructions(m_output_file.get(), "    add t0, t1, t0\n");
        dumpInstructions(m_output_file.get(), end_format);
    }
    else if (strcmp(p_bin_op.getOpCString(), "-") == 0) {
        dumpInstructions(m_output_file.get(), "    sub t0, t1, t0\n");
        dumpInstructions(m_output_file.get(), end_format);
    }
    else if (strcmp(p_bin_op.getOpCString(), "*") == 0) {
        dumpInstructions(m_output_file.get(), "    mul t0, t1, t0\n");
        dumpInstructions(m_output_file.get(), end_format);
    }
    else if (strcmp(p_bin_op.getOpCString(), "/") == 0) {
        dumpInstructions(m_output_file.get(), "    div t0, t1, t0\n");
        dumpInstructions(m_output_file.get(), end_format);
    }
    else if (strcmp(p_bin_op.getOpCString(), "mod") == 0) {
        dumpInstructions(m_output_file.get(), "    rem t0, t1, t0\n");
        dumpInstructions(m_output_file.get(), end_format);
    }

    else if (strcmp(p_bin_op.getOpCString(), "<") == 0) {
        dumpInstructions(m_output_file.get(), "    bge t1, t0, L%d\n", label + 1);
    }
    else if (strcmp(p_bin_op.getOpCString(), "<=") == 0) {
        dumpInstructions(m_output_file.get(), "    bgt t1, t0, L%d\n", label + 1);
    }
    else if (strcmp(p_bin_op.getOpCString(), ">") == 0) {
        dumpInstructions(m_output_file.get(), "    ble t1, t0, L%d\n", label + 1);
    }
    else if (strcmp(p_bin_op.getOpCString(), ">=") == 0) {
        dumpInstructions(m_output_file.get(), "    blt t1, t0, L%d\n", label + 1);
    }
    else if (strcmp(p_bin_op.getOpCString(), "=") == 0) {
        dumpInstructions(m_output_file.get(), "    bne t1, t0, L%d\n", label + 1);
    }
    else if (strcmp(p_bin_op.getOpCString(), "<>") == 0) {
        dumpInstructions(m_output_file.get(), "    beq t1, t0, L%d\n", label + 1);
    }
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op)
{
    // op++;
    p_un_op.visitChildNodes(*this);
    // op--;

    constexpr const char *const format = "    lw t0, 0(sp)\n"
                                         "    addi sp, sp, 4\n";
    constexpr const char *const end_format = "    addi sp, sp, -4\n"
                                             "    sw t0, 0(sp)\n";

    dumpInstructions(m_output_file.get(), format);
    if (strcmp(p_un_op.getOpCString(), "neg") == 0) {
        dumpInstructions(m_output_file.get(), "    neg t0, t0\n");
        dumpInstructions(m_output_file.get(), end_format);
    }
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation)
{
    dumpInstructions(m_output_file.get(), "\n    # function invocation\n");

    func_invo++;
    p_func_invocation.visitChildNodes(*this);
    func_invo--;

    constexpr const char *const format_a = "    lw a%d, 0(sp)\n"
                                           "    addi sp, sp, 4\n";
    constexpr const char *const format = "    jal ra, %s\n"
                                         "    mv t0, a0\n"
                                         "    addi sp, sp, -4\n"
                                         "    sw t0, 0(sp)\n";

    for (unsigned int i = 0; i < p_func_invocation.getArguments().size(); i++)
        dumpInstructions(m_output_file.get(), format_a, i % 8);

    dumpInstructions(m_output_file.get(), format, p_func_invocation.getNameCString());
}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref)
{

    if (SymbolEntry *entry = m_symbol_manager_ptr->lookup(p_variable_ref.getName())) {
        // local variables, local constants
        if (entry->getLevel() != 0 && (entry->getKind() == SymbolEntry::KindEnum::kVariableKind ||
                                       entry->getKind() == SymbolEntry::KindEnum::kConstantKind ||
                                       entry->getKind() == SymbolEntry::KindEnum::kLoopVarKind)) {
            if ((lhs || read) && !op && !func_invo) {
                constexpr const char *const format = "    addi t0, s0, -%d\n"
                                                     "    addi sp, sp, -4\n"
                                                     "    sw t0, 0(sp)\n";
                dumpInstructions(m_output_file.get(), format, entry->getOffset());
            }
            else {
                constexpr const char *const format = "    lw t0, -%d(s0)\n"
                                                     "    addi sp, sp, -4\n"
                                                     "    sw t0, 0(sp)\n";
                dumpInstructions(m_output_file.get(), format, entry->getOffset());
            }
        }

        else if (entry->getKind() == SymbolEntry::KindEnum::kParameterKind) {
            constexpr const char *const format = "    lw t0, -%d(s0)\n"
                                                 "    addi sp, sp, -4\n"
                                                 "    sw t0, 0(sp)\n";
            dumpInstructions(m_output_file.get(), format, entry->getOffset());
        }

        // global variables, global constants
        else if (entry->getLevel() == 0 && (entry->getKind() == SymbolEntry::KindEnum::kVariableKind ||
                                            entry->getKind() == SymbolEntry::KindEnum::kConstantKind)) {

            if ((lhs || read) && !op && !func_invo) {

                constexpr const char *const format = "    la t0, %s\n"
                                                     "    addi sp, sp, -4\n"
                                                     "    sw t0, 0(sp)\n";
                dumpInstructions(m_output_file.get(), format, entry->getNameCString());
            }
            else {
                constexpr const char *const format = "    la t0, %s\n"
                                                     "    lw t1, 0(t0)\n"
                                                     "    mv t0, t1\n"
                                                     "    addi sp, sp, -4\n"
                                                     "    sw t0, 0(sp)\n";
                dumpInstructions(m_output_file.get(), format, entry->getNameCString());
            }
        }
    }
}

void CodeGenerator::visit(AssignmentNode &p_assignment)
{
    dumpInstructions(m_output_file.get(), "\n    # assignment\n");

    lhs = true;
    p_assignment.visitChildNodes(*this);
    lhs = false;

    constexpr const char *const format = "    lw t0, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    lw t1, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    sw t0, 0(t1)    # save the value\n";
    dumpInstructions(m_output_file.get(), format);

    if (for_assign_condition) {
        constexpr const char *const format = "L%d:\n"
                                             "    lw t0, -%d(s0)\n"
                                             "    addi sp, sp, -4\n"
                                             "    sw t0, 0(sp)\n";
        // dumpInstructions(m_output_file.get(), format, label++, local_address_offset);
        dumpInstructions(m_output_file.get(), format, pseudo_label, local_address_offset);
        for_assign_condition--;
    }
}

void CodeGenerator::visit(ReadNode &p_read)
{
    dumpInstructions(m_output_file.get(), "\n    # read\n");

    read = true;
    p_read.visitChildNodes(*this);
    read = false;

    constexpr const char *const format = "    jal ra, readInt\n"
                                         "    lw t0, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    sw a0, 0(t0)\n";
    dumpInstructions(m_output_file.get(), format);
}

void CodeGenerator::visit(IfNode &p_if)
{
    dumpInstructions(m_output_file.get(), "\n    # condition if\n");

    if_condition++;
    else_condition += p_if.hasElseBody();
    p_if.visitChildNodes(*this);
    if_condition--;

    dumpInstructions(m_output_file.get(), "L%d:", label++);
}

void CodeGenerator::visit(WhileNode &p_while)
{
    dumpInstructions(m_output_file.get(), "\nL%d:\n", label++);
    dumpInstructions(m_output_file.get(), "    # condition while\n");

    while_condition++;
    p_while.visitChildNodes(*this);
    while_condition--;

    dumpInstructions(m_output_file.get(), "L%d:", label++);
}

void CodeGenerator::visit(ForNode &p_for)
{
    // if (for_condition) {
    //     return;
    // }

    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(p_for.getSymbolTable());

    dumpInstructions(m_output_file.get(), "\n    # condition for\n");

    pseudo_label = label;
    label_group.push_back(label);
    label += 3;

    for_condition++;
    for_assign_condition++;
    p_for.visitChildNodes(*this);
    for_condition--;
    constexpr const char *const format = "    addi t0, s0, -%d\n"
                                         "    addi sp, sp, -4\n"
                                         "    sw t0, 0(sp)\n"
                                         "    lw t0, -%d(s0)\n"
                                         "    addi sp, sp, -4\n"
                                         "    sw t0, 0(sp)\n"
                                         "    li t0, 1\n"
                                         "    addi sp, sp, -4\n"
                                         "    sw t0, 0(sp)\n"
                                         "    lw t0, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    lw t1, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    add t0, t1, t0\n"
                                         "    addi sp, sp, -4\n"
                                         "    sw t0, 0(sp)\n"
                                         "    lw t0, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    lw t1, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    sw t0, 0(t1)\n"
                                         "    j L%d\n"
                                         "L%d:\n";
    dumpInstructions(m_output_file.get(), format, local_address_offset, local_address_offset, pseudo_label,
                     pseudo_label + 2);

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_for.getSymbolTable());

    local_address_offset -= 4;
    label_group.pop_back();
    pseudo_label = (label_group.empty()) ? 0 : label_group.back();
}

void CodeGenerator::visit(ReturnNode &p_return)
{
    dumpInstructions(m_output_file.get(), "\n    # return\n");

    p_return.visitChildNodes(*this);

    constexpr const char *const format = "    lw a0, 0(sp)\n"
                                         "    addi sp, sp, 4\n"
                                         "    mv a0, t0\n";
    dumpInstructions(m_output_file.get(), format);
}
