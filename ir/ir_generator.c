/*
 * MODULE 6: INTERMEDIATE REPRESENTATION (TAC Generator)
 * Mini-Compiler Project - CS-346 Compiler Construction
 *
 * Generates Three-Address Code from AST.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/common.h"

/* IR Generator state */
typedef struct {
    TACCode *tac;
    SymbolTable *symbols;
    char current_function[MAX_TOKEN_LEN];
} IRGenerator;

/* Forward declarations */
static char* generate_expression(IRGenerator *ir, ASTNode *node);
static void generate_statement(IRGenerator *ir, ASTNode *node);
static void generate_block(IRGenerator *ir, ASTNode *node);
static void generate_function(IRGenerator *ir, ASTNode *node);

/* Create IR generator */
IRGenerator* create_ir_generator(SymbolTable *symbols) {
    IRGenerator *ir = (IRGenerator *)malloc(sizeof(IRGenerator));
    ir->tac = create_tac();
    ir->symbols = symbols;
    ir->current_function[0] = '\0';
    return ir;
}

/* Generate a new temporary variable name */
static char* gen_temp(IRGenerator *ir) {
    return new_temp(ir->tac);
}

/* Generate a new label */
static char* gen_label(IRGenerator *ir) {
    return new_label(ir->tac);
}

/* Emit TAC instruction with label field */
static void emit_with_label(IRGenerator *ir, TACOpType op, const char *result,
                            const char *arg1, const char *arg2, const char *label) {
    if (ir->tac->count >= MAX_TAC_INSTRUCTIONS) return;

    TACInstruction *instr = &ir->tac->instructions[ir->tac->count++];
    instr->op = op;

    if (result) strncpy(instr->result, result, MAX_OPERAND_LEN - 1);
    else instr->result[0] = '\0';

    if (arg1) strncpy(instr->arg1, arg1, MAX_OPERAND_LEN - 1);
    else instr->arg1[0] = '\0';

    if (arg2) strncpy(instr->arg2, arg2, MAX_OPERAND_LEN - 1);
    else instr->arg2[0] = '\0';

    if (label) strncpy(instr->label, label, MAX_LABEL_LEN - 1);
    else instr->label[0] = '\0';

    instr->line_num = ir->tac->count;
    instr->is_leader = 0;
    instr->block_id = -1;
    instr->is_dead = 0;
}

/* Map AST operator to TAC operator */
static TACOpType get_tac_op(const char *op) {
    if (strcmp(op, "+") == 0) return OP_ADD;
    if (strcmp(op, "-") == 0) return OP_SUB;
    if (strcmp(op, "*") == 0) return OP_MUL;
    if (strcmp(op, "/") == 0) return OP_DIV;
    if (strcmp(op, "%") == 0) return OP_MOD;
    if (strcmp(op, "^") == 0) return OP_MUL;  /* Use multiply for power (simplified) */
    if (strcmp(op, "<") == 0) return OP_LT;
    if (strcmp(op, "<=") == 0) return OP_LE;
    if (strcmp(op, ">") == 0) return OP_GT;
    if (strcmp(op, ">=") == 0) return OP_GE;
    if (strcmp(op, "==") == 0) return OP_EQ;
    if (strcmp(op, "!=") == 0) return OP_NE;
    if (strcmp(op, "&&") == 0) return OP_AND;
    if (strcmp(op, "||") == 0) return OP_OR;
    if (strcmp(op, "!") == 0) return OP_NOT;
    return OP_NOP;
}

/* Generate code for expression, return temp holding result */
static char* generate_expression(IRGenerator *ir, ASTNode *node) {
    if (!node) return NULL;

    static char result[MAX_OPERAND_LEN];

    switch (node->type) {
        case AST_INT_LITERAL:
        case AST_FLOAT_LITERAL:
            strcpy(result, node->value);
            return result;

        case AST_IDENTIFIER:
            strcpy(result, node->value);
            return result;

        case AST_BINARY_OP: {
            char *left = generate_expression(ir, node->children[0]);
            char left_copy[MAX_OPERAND_LEN];
            strcpy(left_copy, left);

            char *right = generate_expression(ir, node->children[1]);
            char right_copy[MAX_OPERAND_LEN];
            strcpy(right_copy, right);

            char *temp = gen_temp(ir);
            strcpy(result, temp);

            /* Handle power operator specially */
            if (strcmp(node->value, "^") == 0) {
                /* For simplicity, emit as function call to power */
                emit_tac(ir->tac, OP_PARAM, NULL, left_copy, NULL);
                emit_tac(ir->tac, OP_PARAM, NULL, right_copy, NULL);
                emit_with_label(ir, OP_CALL, result, "pow", "2", NULL);
            } else {
                TACOpType op = get_tac_op(node->value);
                emit_tac(ir->tac, op, result, left_copy, right_copy);
            }
            return result;
        }

        case AST_UNARY_OP: {
            char *operand = generate_expression(ir, node->children[0]);
            char operand_copy[MAX_OPERAND_LEN];
            strcpy(operand_copy, operand);

            char *temp = gen_temp(ir);
            strcpy(result, temp);

            if (strcmp(node->value, "-") == 0) {
                emit_tac(ir->tac, OP_UMINUS, result, operand_copy, NULL);
            } else if (strcmp(node->value, "!") == 0) {
                emit_tac(ir->tac, OP_NOT, result, operand_copy, NULL);
            }
            return result;
        }

        case AST_CALL: {
            /* Generate arguments */
            for (int i = 0; i < node->child_count; i++) {
                char *arg = generate_expression(ir, node->children[i]);
                char arg_copy[MAX_OPERAND_LEN];
                strcpy(arg_copy, arg);
                emit_tac(ir->tac, OP_PARAM, NULL, arg_copy, NULL);
            }

            char *temp = gen_temp(ir);
            strcpy(result, temp);

            char param_count[16];
            snprintf(param_count, sizeof(param_count), "%d", node->child_count);
            emit_with_label(ir, OP_CALL, result, node->value, param_count, NULL);
            return result;
        }

        case AST_ARRAY_ACCESS: {
            char *index = generate_expression(ir, node->children[0]);
            char index_copy[MAX_OPERAND_LEN];
            strcpy(index_copy, index);

            char *temp = gen_temp(ir);
            strcpy(result, temp);
            emit_tac(ir->tac, OP_ARRAY_REF, result, node->value, index_copy);
            return result;
        }

        case AST_MATH_FUNC: {
            char *arg = generate_expression(ir, node->children[0]);
            char arg_copy[MAX_OPERAND_LEN];
            strcpy(arg_copy, arg);

            emit_tac(ir->tac, OP_PARAM, NULL, arg_copy, NULL);

            char *temp = gen_temp(ir);
            strcpy(result, temp);
            emit_with_label(ir, OP_CALL, result, node->value, "1", NULL);
            return result;
        }

        default:
            result[0] = '\0';
            return result;
    }
}

/* Generate code for variable declaration */
static void generate_var_decl(IRGenerator *ir, ASTNode *node) {
    /* Find initialization expression (skip array size if present) */
    int init_idx = 0;
    if (node->child_count > 0 && node->children[0]->type == AST_INT_LITERAL) {
        /* This is array size, check for init at next index */
        init_idx = 1;
    }

    if (node->child_count > init_idx) {
        char *value = generate_expression(ir, node->children[init_idx]);
        emit_tac(ir->tac, OP_ASSIGN, node->value, value, NULL);
    }
}

/* Generate code for assignment */
static void generate_assignment(IRGenerator *ir, ASTNode *node) {
    int value_idx = node->child_count - 1;
    if (value_idx < 0) return;

    char *value = generate_expression(ir, node->children[value_idx]);
    char value_copy[MAX_OPERAND_LEN];
    strcpy(value_copy, value);

    /* Array assignment */
    if (node->child_count > 1) {
        char *index = generate_expression(ir, node->children[0]);
        char index_copy[MAX_OPERAND_LEN];
        strcpy(index_copy, index);
        emit_tac(ir->tac, OP_ARRAY_ASSIGN, node->value, index_copy, value_copy);
    } else {
        emit_tac(ir->tac, OP_ASSIGN, node->value, value_copy, NULL);
    }
}

/* Generate code for if statement */
static void generate_if(IRGenerator *ir, ASTNode *node) {
    if (node->child_count < 2) return;

    char *cond = generate_expression(ir, node->children[0]);
    char cond_copy[MAX_OPERAND_LEN];
    strcpy(cond_copy, cond);

    char *else_label = gen_label(ir);
    char else_copy[MAX_LABEL_LEN];
    strcpy(else_copy, else_label);

    char *end_label = gen_label(ir);
    char end_copy[MAX_LABEL_LEN];
    strcpy(end_copy, end_label);

    /* iffalse cond goto else_label */
    emit_with_label(ir, OP_IFFALSE, NULL, cond_copy, NULL, else_copy);

    /* Then block */
    generate_block(ir, node->children[1]);

    /* Has else block? */
    if (node->child_count > 2) {
        /* goto end_label */
        emit_with_label(ir, OP_GOTO, NULL, end_copy, NULL, NULL);

        /* else_label: */
        emit_tac(ir->tac, OP_LABEL, NULL, else_copy, NULL);

        /* Else block */
        generate_block(ir, node->children[2]);

        /* end_label: */
        emit_tac(ir->tac, OP_LABEL, NULL, end_copy, NULL);
    } else {
        /* else_label: */
        emit_tac(ir->tac, OP_LABEL, NULL, else_copy, NULL);
    }
}

/* Generate code for while statement */
static void generate_while(IRGenerator *ir, ASTNode *node) {
    if (node->child_count < 2) return;

    char *start_label = gen_label(ir);
    char start_copy[MAX_LABEL_LEN];
    strcpy(start_copy, start_label);

    char *end_label = gen_label(ir);
    char end_copy[MAX_LABEL_LEN];
    strcpy(end_copy, end_label);

    /* start_label: */
    emit_tac(ir->tac, OP_LABEL, NULL, start_copy, NULL);

    /* Evaluate condition */
    char *cond = generate_expression(ir, node->children[0]);
    char cond_copy[MAX_OPERAND_LEN];
    strcpy(cond_copy, cond);

    /* iffalse cond goto end_label */
    emit_with_label(ir, OP_IFFALSE, NULL, cond_copy, NULL, end_copy);

    /* Body */
    generate_block(ir, node->children[1]);

    /* goto start_label */
    emit_with_label(ir, OP_GOTO, NULL, start_copy, NULL, NULL);

    /* end_label: */
    emit_tac(ir->tac, OP_LABEL, NULL, end_copy, NULL);
}

/* Generate code for for statement */
static void generate_for(IRGenerator *ir, ASTNode *node) {
    if (node->child_count < 4) return;

    /* Init */
    generate_statement(ir, node->children[0]);

    char *start_label = gen_label(ir);
    char start_copy[MAX_LABEL_LEN];
    strcpy(start_copy, start_label);

    char *end_label = gen_label(ir);
    char end_copy[MAX_LABEL_LEN];
    strcpy(end_copy, end_label);

    /* start_label: */
    emit_tac(ir->tac, OP_LABEL, NULL, start_copy, NULL);

    /* Condition */
    char *cond = generate_expression(ir, node->children[1]);
    char cond_copy[MAX_OPERAND_LEN];
    strcpy(cond_copy, cond);

    /* iffalse cond goto end_label */
    emit_with_label(ir, OP_IFFALSE, NULL, cond_copy, NULL, end_copy);

    /* Body */
    generate_block(ir, node->children[3]);

    /* Update */
    generate_expression(ir, node->children[2]);

    /* goto start_label */
    emit_with_label(ir, OP_GOTO, NULL, start_copy, NULL, NULL);

    /* end_label: */
    emit_tac(ir->tac, OP_LABEL, NULL, end_copy, NULL);
}

/* Generate code for return statement */
static void generate_return(IRGenerator *ir, ASTNode *node) {
    if (node->child_count > 0) {
        char *value = generate_expression(ir, node->children[0]);
        emit_tac(ir->tac, OP_RETURN, NULL, value, NULL);
    } else {
        emit_tac(ir->tac, OP_RETURN, NULL, NULL, NULL);
    }
}

/* Generate code for statement */
static void generate_statement(IRGenerator *ir, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_VAR_DECL:
            generate_var_decl(ir, node);
            break;
        case AST_ASSIGNMENT:
            generate_assignment(ir, node);
            break;
        case AST_IF_STMT:
            generate_if(ir, node);
            break;
        case AST_WHILE_STMT:
            generate_while(ir, node);
            break;
        case AST_FOR_STMT:
            generate_for(ir, node);
            break;
        case AST_RETURN_STMT:
            generate_return(ir, node);
            break;
        case AST_BLOCK:
            generate_block(ir, node);
            break;
        case AST_CALL:
            generate_expression(ir, node);
            break;
        default:
            break;
    }
}

/* Generate code for block */
static void generate_block(IRGenerator *ir, ASTNode *node) {
    if (!node) return;

    for (int i = 0; i < node->child_count; i++) {
        generate_statement(ir, node->children[i]);
    }
}

/* Generate code for function */
static void generate_function(IRGenerator *ir, ASTNode *node) {
    strcpy(ir->current_function, node->value);

    /* function label */
    emit_tac(ir->tac, OP_FUNC, NULL, node->value, NULL);

    /* Generate body */
    if (node->child_count > 1) {
        generate_block(ir, node->children[1]);
    }

    /* Add implicit return for void functions */
    if (strcmp(node->data_type, "void") == 0) {
        emit_tac(ir->tac, OP_RETURN, NULL, NULL, NULL);
    }
}

/* Generate code for program */
static void generate_program(IRGenerator *ir, ASTNode *node) {
    if (!node) return;

    for (int i = 0; i < node->child_count; i++) {
        ASTNode *child = node->children[i];
        if (child->type == AST_FUNCTION) {
            generate_function(ir, child);
        } else {
            generate_statement(ir, child);
        }
    }
}

/* Main IR generation function */
TACCode* generate_ir(ASTNode *ast, SymbolTable *symbols) {
    IRGenerator *ir = create_ir_generator(symbols);

    printf("\n--- IR Generation ---\n");
    generate_program(ir, ast);

    TACCode *result = ir->tac;
    free(ir);

    return result;
}

/* Export TAC to file */
void export_tac_to_file(TACCode *tac, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return;
    }

    fprintf(f, "; Three-Address Code\n");
    fprintf(f, "; Instructions: %d\n", tac->count);
    fprintf(f, "; Temps: %d, Labels: %d\n\n", tac->temp_count, tac->label_count);

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        switch (instr->op) {
            case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
            case OP_LT: case OP_LE: case OP_GT: case OP_GE: case OP_EQ: case OP_NE:
            case OP_AND: case OP_OR:
                fprintf(f, "    %s = %s %s %s\n", instr->result, instr->arg1,
                       tac_op_to_string(instr->op), instr->arg2);
                break;

            case OP_ASSIGN:
            case OP_COPY:
                fprintf(f, "    %s = %s\n", instr->result, instr->arg1);
                break;

            case OP_UMINUS:
            case OP_NOT:
                fprintf(f, "    %s = %s %s\n", instr->result,
                       tac_op_to_string(instr->op), instr->arg1);
                break;

            case OP_ARRAY_REF:
                fprintf(f, "    %s = %s[%s]\n", instr->result, instr->arg1, instr->arg2);
                break;

            case OP_ARRAY_ASSIGN:
                fprintf(f, "    %s[%s] = %s\n", instr->result, instr->arg1, instr->arg2);
                break;

            case OP_GOTO:
                fprintf(f, "    goto %s\n", instr->label[0] ? instr->label : instr->arg1);
                break;

            case OP_IFGOTO:
                fprintf(f, "    if %s goto %s\n", instr->arg1,
                       instr->label[0] ? instr->label : instr->arg2);
                break;

            case OP_IFFALSE:
                fprintf(f, "    iffalse %s goto %s\n", instr->arg1,
                       instr->label[0] ? instr->label : instr->arg2);
                break;

            case OP_PARAM:
                fprintf(f, "    param %s\n", instr->arg1);
                break;

            case OP_CALL:
                if (instr->result[0]) {
                    fprintf(f, "    %s = call %s, %s\n", instr->result, instr->arg1, instr->arg2);
                } else {
                    fprintf(f, "    call %s, %s\n", instr->arg1, instr->arg2);
                }
                break;

            case OP_RETURN:
                if (instr->arg1[0]) {
                    fprintf(f, "    return %s\n", instr->arg1);
                } else {
                    fprintf(f, "    return\n");
                }
                break;

            case OP_FUNC:
                fprintf(f, "\nfunction %s:\n", instr->arg1);
                break;

            case OP_LABEL:
                fprintf(f, "%s:\n", instr->arg1);
                break;

            case OP_NOP:
                fprintf(f, "    nop\n");
                break;

            default:
                fprintf(f, "    ; unknown\n");
        }
    }

    fclose(f);
    printf("TAC exported to: %s\n", filename);
}

/* Standalone main for testing */
#ifdef IR_STANDALONE
int main(int argc, char *argv[]) {
    char source[100000];
    int len = 0;

    printf("=== MODULE 6: IR GENERATION ===\n\n");

    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
        len = fread(source, 1, sizeof(source) - 1, f);
        source[len] = '\0';
        fclose(f);
    } else {
        printf("Reading from stdin...\n");
        len = fread(source, 1, sizeof(source) - 1, stdin);
        source[len] = '\0';
    }

    /* Tokenize, parse, analyze, generate */
    extern TokenStream* tokenize(const char *source);
    extern ASTNode* parse(TokenStream *tokens, int *error_count, char *error_msg);
    extern int semantic_analyze(ASTNode *ast, SymbolTable **symbols,
                                int *errors, int *warnings, char *err_msg, char *warn_msg);

    TokenStream *tokens = tokenize(source);
    int parse_errors;
    char error_msg[5000];
    ASTNode *ast = parse(tokens, &parse_errors, error_msg);

    if (parse_errors > 0) {
        printf("Parse errors:\n%s", error_msg);
        return 1;
    }

    SymbolTable *symbols;
    int sem_errors, sem_warnings;
    char sem_err[10000], sem_warn[5000];
    semantic_analyze(ast, &symbols, &sem_errors, &sem_warnings, sem_err, sem_warn);

    if (sem_errors > 0) {
        printf("Semantic errors:\n%s", sem_err);
        return 1;
    }

    TACCode *tac = generate_ir(ast, symbols);
    print_tac(tac);

    export_tac_to_file(tac, "output.tac");

    free_tac(tac);
    free_symbol_table(symbols);
    free_ast(ast);
    free(tokens);

    return 0;
}
#endif
