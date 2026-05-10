/*
 * COMMON IMPLEMENTATION - Shared Functions
 * Mini-Compiler Project - CS-346 Compiler Construction
 */

#include "common.h"

/* ============================================================
 * TOKEN FUNCTIONS
 * ============================================================ */
const char* token_type_to_string(MiniTokenType type) {
    switch (type) {
        case TOK_INT: return "INT";
        case TOK_FLOAT: return "FLOAT";
        case TOK_CHAR: return "CHAR";
        case TOK_VOID: return "VOID";
        case TOK_BOOL: return "BOOL";
        case TOK_IF: return "IF";
        case TOK_ELSE: return "ELSE";
        case TOK_WHILE: return "WHILE";
        case TOK_FOR: return "FOR";
        case TOK_RETURN: return "RETURN";
        case TOK_FUNCTION: return "FUNCTION";
        case TOK_PROCEDURE: return "PROCEDURE";
        case TOK_BEGIN: return "BEGIN";
        case TOK_END: return "END";
        case TOK_THEN: return "THEN";
        case TOK_DO: return "DO";
        case TOK_INT_LIT: return "INT_LIT";
        case TOK_FLOAT_LIT: return "FLOAT_LIT";
        case TOK_STRING_LIT: return "STRING_LIT";
        case TOK_CHAR_LIT: return "CHAR_LIT";
        case TOK_IDENTIFIER: return "IDENTIFIER";
        case TOK_PLUS: return "PLUS";
        case TOK_MINUS: return "MINUS";
        case TOK_STAR: return "STAR";
        case TOK_SLASH: return "SLASH";
        case TOK_MOD: return "MOD";
        case TOK_POWER: return "POWER";
        case TOK_ASSIGN: return "ASSIGN";
        case TOK_EQ: return "EQ";
        case TOK_NE: return "NE";
        case TOK_LT: return "LT";
        case TOK_LE: return "LE";
        case TOK_GT: return "GT";
        case TOK_GE: return "GE";
        case TOK_AND: return "AND";
        case TOK_OR: return "OR";
        case TOK_NOT: return "NOT";
        case TOK_LPAREN: return "LPAREN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_LBRACE: return "LBRACE";
        case TOK_RBRACE: return "RBRACE";
        case TOK_LBRACKET: return "LBRACKET";
        case TOK_RBRACKET: return "RBRACKET";
        case TOK_SEMICOLON: return "SEMICOLON";
        case TOK_COMMA: return "COMMA";
        case TOK_COLON: return "COLON";
        case TOK_LOG: return "LOG";
        case TOK_EXP: return "EXP";
        case TOK_SQRT: return "SQRT";
        case TOK_SIN: return "SIN";
        case TOK_COS: return "COS";
        case TOK_TAN: return "TAN";
        case TOK_EOF: return "EOF";
        case TOK_NEWLINE: return "NEWLINE";
        case TOK_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/* ============================================================
 * AST FUNCTIONS
 * ============================================================ */
ASTNode* create_ast_node(ASTNodeType type, const char *value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) return NULL;

    node->type = type;
    if (value) {
        strncpy(node->value, value, MAX_TOKEN_LEN - 1);
        node->value[MAX_TOKEN_LEN - 1] = '\0';
    } else {
        node->value[0] = '\0';
    }
    node->data_type[0] = '\0';
    node->line = 0;
    node->child_count = 0;
    node->next = NULL;

    for (int i = 0; i < 10; i++) {
        node->children[i] = NULL;
    }

    return node;
}

void add_ast_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return;
    if (parent->child_count < 10) {
        parent->children[parent->child_count++] = child;
    }
}

static const char* ast_type_to_string(ASTNodeType type) {
    switch (type) {
        case AST_PROGRAM: return "Program";
        case AST_FUNCTION: return "Function";
        case AST_VAR_DECL: return "VarDecl";
        case AST_ASSIGNMENT: return "Assignment";
        case AST_BINARY_OP: return "BinaryOp";
        case AST_UNARY_OP: return "UnaryOp";
        case AST_IF_STMT: return "IfStmt";
        case AST_WHILE_STMT: return "WhileStmt";
        case AST_FOR_STMT: return "ForStmt";
        case AST_RETURN_STMT: return "ReturnStmt";
        case AST_BLOCK: return "Block";
        case AST_CALL: return "Call";
        case AST_IDENTIFIER: return "Identifier";
        case AST_INT_LITERAL: return "IntLiteral";
        case AST_FLOAT_LITERAL: return "FloatLiteral";
        case AST_ARRAY_ACCESS: return "ArrayAccess";
        case AST_MATH_FUNC: return "MathFunc";
        case AST_PARAM: return "Param";
        case AST_PARAM_LIST: return "ParamList";
        default: return "Unknown";
    }
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");

    printf("%s", ast_type_to_string(node->type));
    if (node->value[0]) {
        printf(": %s", node->value);
    }
    if (node->data_type[0]) {
        printf(" [%s]", node->data_type);
    }
    printf("\n");

    for (int i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], indent + 1);
    }

    if (node->next) {
        print_ast(node->next, indent);
    }
}

void free_ast(ASTNode *node) {
    if (!node) return;

    for (int i = 0; i < node->child_count; i++) {
        free_ast(node->children[i]);
    }

    if (node->next) {
        free_ast(node->next);
    }

    free(node);
}

/* ============================================================
 * SYMBOL TABLE FUNCTIONS
 * ============================================================ */
SymbolTable* create_symbol_table(void) {
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    if (table) {
        table->count = 0;
    }
    return table;
}

void add_symbol(SymbolTable *table, const char *name, DataType type, int scope) {
    if (!table || table->count >= MAX_SYMBOLS) return;

    Symbol *sym = &table->symbols[table->count++];
    strncpy(sym->name, name, MAX_TOKEN_LEN - 1);
    sym->type = type;
    strncpy(sym->type_name, data_type_to_string(type), 31);
    sym->scope_level = scope;
    sym->is_function = 0;
    sym->is_array = 0;
    sym->array_size = 0;
    sym->param_count = 0;
    sym->line_declared = 0;
    strcpy(sym->scope_name, "global");
}

Symbol* lookup_symbol(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;

    for (int i = table->count - 1; i >= 0; i--) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

const char* data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_CHAR: return "char";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
        case TYPE_ARRAY: return "array";
        case TYPE_FUNCTION: return "function";
        case TYPE_STRING: return "string";
        default: return "unknown";
    }
}

DataType string_to_data_type(const char *str) {
    if (!str) return TYPE_UNKNOWN;
    if (strcmp(str, "int") == 0) return TYPE_INT;
    if (strcmp(str, "float") == 0) return TYPE_FLOAT;
    if (strcmp(str, "char") == 0) return TYPE_CHAR;
    if (strcmp(str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(str, "void") == 0) return TYPE_VOID;
    return TYPE_UNKNOWN;
}

void print_symbol_table(SymbolTable *table) {
    if (!table) return;

    printf("\n========== SYMBOL TABLE ==========\n");
    printf("%-20s %-12s %-8s %-15s\n", "Name", "Type", "Scope", "Attributes");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < table->count; i++) {
        Symbol *s = &table->symbols[i];
        printf("%-20s %-12s %-8d ", s->name, s->type_name, s->scope_level);

        if (s->is_function) printf("function ");
        if (s->is_array) printf("array[%d] ", s->array_size);
        printf("\n");
    }
    printf("==================================\n");
}

void free_symbol_table(SymbolTable *table) {
    if (table) free(table);
}

/* ============================================================
 * TAC FUNCTIONS
 * ============================================================ */
TACCode* create_tac(void) {
    TACCode *tac = (TACCode *)malloc(sizeof(TACCode));
    if (tac) {
        tac->count = 0;
        tac->temp_count = 0;
        tac->label_count = 0;
    }
    return tac;
}

char* new_temp(TACCode *tac) {
    static char temp[MAX_OPERAND_LEN];
    snprintf(temp, MAX_OPERAND_LEN, "t%d", tac->temp_count++);
    return temp;
}

char* new_label(TACCode *tac) {
    static char label[MAX_LABEL_LEN];
    snprintf(label, MAX_LABEL_LEN, "L%d", tac->label_count++);
    return label;
}

void emit_tac(TACCode *tac, TACOpType op, const char *result, const char *arg1, const char *arg2) {
    if (!tac || tac->count >= MAX_TAC_INSTRUCTIONS) return;

    TACInstruction *instr = &tac->instructions[tac->count++];
    instr->op = op;

    if (result) strncpy(instr->result, result, MAX_OPERAND_LEN - 1);
    else instr->result[0] = '\0';

    if (arg1) strncpy(instr->arg1, arg1, MAX_OPERAND_LEN - 1);
    else instr->arg1[0] = '\0';

    if (arg2) strncpy(instr->arg2, arg2, MAX_OPERAND_LEN - 1);
    else instr->arg2[0] = '\0';

    instr->label[0] = '\0';
    instr->line_num = tac->count;
    instr->is_leader = 0;
    instr->block_id = -1;
    instr->is_dead = 0;
}

const char* tac_op_to_string(TACOpType op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_ASSIGN: return "=";
        case OP_UMINUS: return "neg";
        case OP_LT: return "<";
        case OP_LE: return "<=";
        case OP_GT: return ">";
        case OP_GE: return ">=";
        case OP_EQ: return "==";
        case OP_NE: return "!=";
        case OP_AND: return "&&";
        case OP_OR: return "||";
        case OP_NOT: return "!";
        case OP_ARRAY_REF: return "[]";
        case OP_ARRAY_ASSIGN: return "[]=";
        case OP_GOTO: return "goto";
        case OP_IFGOTO: return "if_goto";
        case OP_IFFALSE: return "iffalse";
        case OP_PARAM: return "param";
        case OP_CALL: return "call";
        case OP_RETURN: return "return";
        case OP_FUNC: return "func";
        case OP_LABEL: return "label";
        case OP_COPY: return "copy";
        case OP_NOP: return "nop";
        default: return "???";
    }
}

void print_tac(TACCode *tac) {
    if (!tac) return;

    printf("\n========== THREE-ADDRESS CODE ==========\n");
    printf("Total Instructions: %d\n", tac->count);
    printf("Temp Variables: %d, Labels: %d\n\n", tac->temp_count, tac->label_count);

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        if (instr->is_dead) {
            printf("[DEAD] ");
        }

        printf("%4d: ", i);

        switch (instr->op) {
            case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
            case OP_LT: case OP_LE: case OP_GT: case OP_GE: case OP_EQ: case OP_NE:
            case OP_AND: case OP_OR:
                printf("%s = %s %s %s\n", instr->result, instr->arg1,
                       tac_op_to_string(instr->op), instr->arg2);
                break;

            case OP_ASSIGN:
            case OP_COPY:
                printf("%s = %s\n", instr->result, instr->arg1);
                break;

            case OP_UMINUS:
            case OP_NOT:
                printf("%s = %s %s\n", instr->result, tac_op_to_string(instr->op), instr->arg1);
                break;

            case OP_ARRAY_REF:
                printf("%s = %s[%s]\n", instr->result, instr->arg1, instr->arg2);
                break;

            case OP_ARRAY_ASSIGN:
                printf("%s[%s] = %s\n", instr->result, instr->arg1, instr->arg2);
                break;

            case OP_GOTO:
                printf("goto %s\n", instr->label[0] ? instr->label : instr->arg1);
                break;

            case OP_IFGOTO:
                printf("if %s goto %s\n", instr->arg1, instr->label[0] ? instr->label : instr->arg2);
                break;

            case OP_IFFALSE:
                printf("iffalse %s goto %s\n", instr->arg1, instr->label[0] ? instr->label : instr->arg2);
                break;

            case OP_PARAM:
                printf("param %s\n", instr->arg1);
                break;

            case OP_CALL:
                if (instr->result[0]) {
                    printf("%s = call %s, %s\n", instr->result, instr->arg1, instr->arg2);
                } else {
                    printf("call %s, %s\n", instr->arg1, instr->arg2);
                }
                break;

            case OP_RETURN:
                if (instr->arg1[0]) {
                    printf("return %s\n", instr->arg1);
                } else {
                    printf("return\n");
                }
                break;

            case OP_FUNC:
                printf("\nfunction %s:\n", instr->arg1);
                break;

            case OP_LABEL:
                printf("%s:\n", instr->arg1);
                break;

            case OP_NOP:
                printf("nop\n");
                break;

            default:
                printf("unknown op\n");
        }
    }
    printf("=========================================\n");
}

void free_tac(TACCode *tac) {
    if (tac) free(tac);
}

/* ============================================================
 * COMPILER STATE
 * ============================================================ */
CompilerState* create_compiler_state(void) {
    CompilerState *state = (CompilerState *)calloc(1, sizeof(CompilerState));
    if (state) {
        state->symbol_table = create_symbol_table();
        state->tac = create_tac();
        state->optimized_tac = create_tac();
    }
    return state;
}

void free_compiler_state(CompilerState *state) {
    if (!state) return;

    if (state->source_code) free(state->source_code);
    if (state->ast) free_ast(state->ast);
    if (state->symbol_table) free_symbol_table(state->symbol_table);
    if (state->tac) free_tac(state->tac);
    if (state->optimized_tac) free_tac(state->optimized_tac);
    if (state->cfg) free(state->cfg);

    free(state);
}
