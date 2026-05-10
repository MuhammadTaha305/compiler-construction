#include "tac.h"

TACCode* create_tac_code() {
    TACCode *tac = (TACCode *)malloc(sizeof(TACCode));
    tac->count = 0;
    tac->temp_var_count = 0;
    tac->label_count = 0;
    return tac;
}

void emit_instruction(TACCode *tac, OpType op, const char *result, const char *op1, const char *op2) {
    if (tac->count >= MAX_TAC_INSTRUCTIONS) {
        fprintf(stderr, "TAC code buffer overflow\n");
        return;
    }

    TACInstruction *instr = &tac->instructions[tac->count++];
    instr->op = op;
    
    if (result) strncpy(instr->result, result, MAX_OPERAND_LEN - 1);
    else instr->result[0] = '\0';
    
    if (op1) strncpy(instr->operand1, op1, MAX_OPERAND_LEN - 1);
    else instr->operand1[0] = '\0';
    
    if (op2) strncpy(instr->operand2, op2, MAX_OPERAND_LEN - 1);
    else instr->operand2[0] = '\0';
    
    instr->label[0] = '\0';
    instr->line_num = tac->count;
}

void emit_instruction_with_label(TACCode *tac, OpType op, const char *result, const char *op1, const char *label) {
    if (tac->count >= MAX_TAC_INSTRUCTIONS) {
        fprintf(stderr, "TAC code buffer overflow\n");
        return;
    }

    TACInstruction *instr = &tac->instructions[tac->count++];
    instr->op = op;
    
    if (result) strncpy(instr->result, result, MAX_OPERAND_LEN - 1);
    else instr->result[0] = '\0';
    
    if (op1) strncpy(instr->operand1, op1, MAX_OPERAND_LEN - 1);
    else instr->operand1[0] = '\0';
    
    instr->operand2[0] = '\0';
    
    if (label) strncpy(instr->label, label, MAX_LABEL_LEN - 1);
    else instr->label[0] = '\0';
    
    instr->line_num = tac->count;
}

void emit_goto(TACCode *tac, const char *label) {
    emit_instruction_with_label(tac, OP_GOTO, NULL, NULL, label);
}

void emit_ifgoto(TACCode *tac, const char *op1, const char *relop, const char *op2, const char *label) {
    if (tac->count >= MAX_TAC_INSTRUCTIONS) {
        fprintf(stderr, "TAC code buffer overflow\n");
        return;
    }

    TACInstruction *instr = &tac->instructions[tac->count++];
    instr->op = OP_IFGOTO;
    
    instr->result[0] = '\0';
    strncpy(instr->operand1, op1, MAX_OPERAND_LEN - 1);
    strncpy(instr->operand2, relop, MAX_OPERAND_LEN - 1);
    strncpy(instr->label, label, MAX_LABEL_LEN - 1);
    instr->line_num = tac->count;
}

void emit_param(TACCode *tac, const char *operand) {
    emit_instruction(tac, OP_PARAM, NULL, operand, NULL);
}

void emit_call(TACCode *tac, const char *result, const char *func_name, int param_count) {
    if (tac->count >= MAX_TAC_INSTRUCTIONS) {
        fprintf(stderr, "TAC code buffer overflow\n");
        return;
    }

    TACInstruction *instr = &tac->instructions[tac->count++];
    instr->op = OP_CALL;
    strncpy(instr->result, result, MAX_OPERAND_LEN - 1);
    strncpy(instr->operand1, func_name, MAX_OPERAND_LEN - 1);
    snprintf(instr->operand2, MAX_OPERAND_LEN - 1, "%d", param_count);
    instr->label[0] = '\0';
    instr->line_num = tac->count;
}

void emit_return(TACCode *tac, const char *operand) {
    emit_instruction(tac, OP_RETURN, NULL, operand, NULL);
}

void emit_function(TACCode *tac, const char *func_name) {
    emit_instruction_with_label(tac, OP_FUNC, NULL, func_name, NULL);
}

void emit_label(TACCode *tac, const char *label_name) {
    emit_instruction_with_label(tac, OP_LABEL, NULL, label_name, NULL);
}

char* new_temp_var(TACCode *tac) {
    static char temp[MAX_OPERAND_LEN];
    snprintf(temp, MAX_OPERAND_LEN, "t%d", tac->temp_var_count++);
    return temp;
}

char* new_label(TACCode *tac) {
    static char label[MAX_LABEL_LEN];
    snprintf(label, MAX_LABEL_LEN, "L%d", tac->label_count++);
    return label;
}

const char* op_to_string(OpType op) {
    switch (op) {
        case OP_ADD:           return "+";
        case OP_SUB:           return "-";
        case OP_MUL:           return "*";
        case OP_DIV:           return "/";
        case OP_MOD:           return "%";
        case OP_ASSIGN:        return "=";
        case OP_UMINUS:        return "neg";
        case OP_LT:            return "<";
        case OP_LE:            return "<=";
        case OP_GT:            return ">";
        case OP_GE:            return ">=";
        case OP_EQ:            return "==";
        case OP_NE:            return "!=";
        case OP_AND:           return "&&";
        case OP_OR:            return "||";
        case OP_NOT:           return "!";
        case OP_ARRAY_REF:     return "a[]";
        case OP_ARRAY_ASSIGN:  return "[]a";
        case OP_GOTO:          return "goto";
        case OP_IFGOTO:        return "ifgoto";
        case OP_IFNOT:         return "ifnot";
        case OP_PARAM:         return "param";
        case OP_CALL:          return "call";
        case OP_RETURN:        return "return";
        case OP_FUNC:          return "func";
        case OP_LABEL:         return "label";
        default:               return "unknown";
    }
}

void print_tac_code(TACCode *tac) {
    printf("\nTHREE-ADDRESS CODE (TAC): \n\n");

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];
        printf("%4d: ", i);

        switch (instr->op) {
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_MOD:
            case OP_LT:
            case OP_LE:
            case OP_GT:
            case OP_GE:
            case OP_EQ:
            case OP_NE:
            case OP_AND:
            case OP_OR:
                printf("%s = %s %s %s\n", instr->result, instr->operand1, op_to_string(instr->op), instr->operand2);
                break;

            case OP_ASSIGN:
                printf("%s = %s\n", instr->result, instr->operand1);
                break;

            case OP_UMINUS:
            case OP_NOT:
                printf("%s = %s%s\n", instr->result, op_to_string(instr->op), instr->operand1);
                break;

            case OP_ARRAY_REF:
                printf("%s = %s[%s]\n", instr->result, instr->operand1, instr->operand2);
                break;

            case OP_ARRAY_ASSIGN:
                printf("%s[%s] = %s\n", instr->result, instr->operand1, instr->operand2);
                break;

            case OP_GOTO:
                printf("goto %s\n", instr->label);
                break;

            case OP_IFGOTO:
                printf("if %s %s %s goto %s\n", instr->operand1, instr->operand2, instr->label, instr->label);
                break;

            case OP_IFNOT:
                printf("ifnot %s goto %s\n", instr->operand1, instr->label);
                break;

            case OP_PARAM:
                printf("param %s\n", instr->operand1);
                break;

            case OP_CALL:
                printf("%s = call %s, %s\n", instr->result, instr->operand1, instr->operand2);
                break;

            case OP_RETURN:
                printf("return %s\n", instr->operand1);
                break;

            case OP_FUNC:
                printf("function %s:\n", instr->operand1);
                break;

            case OP_LABEL:
                printf("%s:\n", instr->operand1);
                break;

            default:
                printf("unknown operation\n");
        }
    }
}

void print_tac_code_detailed(TACCode *tac) {
    printf("\nTHREE-ADDRESS CODE (TAC) - DETAILED\n\n");
    printf("Total Instructions: %d\n", tac->count);
    printf("Temporary Variables Used: %d\n", tac->temp_var_count);
    printf("Labels Used: %d\n\n", tac->label_count);

    print_tac_code(tac);
}

void export_tac_code(TACCode *tac, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return;
    }

    fprintf(f, "--THREE-ADDRESS CODE--\n");

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];
        fprintf(f, "%4d: ", i);

        switch (instr->op) {
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_MOD:
            case OP_LT:
            case OP_LE:
            case OP_GT:
            case OP_GE:
            case OP_EQ:
            case OP_NE:
            case OP_AND:
            case OP_OR:
                fprintf(f, "%s = %s %s %s\n", instr->result, instr->operand1, op_to_string(instr->op), instr->operand2);
                break;

            case OP_ASSIGN:
                fprintf(f, "%s = %s\n", instr->result, instr->operand1);
                break;

            case OP_UMINUS:
            case OP_NOT:
                fprintf(f, "%s = %s%s\n", instr->result, op_to_string(instr->op), instr->operand1);
                break;

            case OP_ARRAY_REF:
                fprintf(f, "%s = %s[%s]\n", instr->result, instr->operand1, instr->operand2);
                break;

            case OP_ARRAY_ASSIGN:
                fprintf(f, "%s[%s] = %s\n", instr->result, instr->operand1, instr->operand2);
                break;

            case OP_GOTO:
                fprintf(f, "goto %s\n", instr->label);
                break;

            case OP_IFGOTO:
                fprintf(f, "if %s %s %s goto %s\n", instr->operand1, instr->operand2, instr->label, instr->label);
                break;

            case OP_PARAM:
                fprintf(f, "param %s\n", instr->operand1);
                break;

            case OP_CALL:
                fprintf(f, "%s = call %s, %s\n", instr->result, instr->operand1, instr->operand2);
                break;

            case OP_RETURN:
                fprintf(f, "return %s\n", instr->operand1);
                break;

            case OP_FUNC:
                fprintf(f, "function %s:\n", instr->operand1);
                break;

            case OP_LABEL:
                fprintf(f, "%s:\n", instr->operand1);
                break;

            default:
                fprintf(f, "unknown operation\n");
        }
    }

    fclose(f);
}

void free_tac_code(TACCode *tac) {
    if (tac) free(tac);
}
