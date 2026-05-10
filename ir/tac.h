#ifndef TAC_H
#define TAC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAC_INSTRUCTIONS 10000
#define MAX_OPERAND_LEN 100
#define MAX_LABEL_LEN 50
#define MAX_TEMP_VARS 1000

typedef enum {
    OP_ADD,        // x = y + z
    OP_SUB,        // x = y - z
    OP_MUL,        // x = y * z
    OP_DIV,        // x = y / z
    OP_MOD,        // x = y % z
    OP_ASSIGN,     // x = y
    OP_UMINUS,     // x = -y
    OP_LT,         // x = y < z
    OP_LE,         // x = y <= z
    OP_GT,         // x = y > z
    OP_GE,         // x = y >= z
    OP_EQ,         // x = y == z
    OP_NE,         // x = y != z
    OP_AND,        // x = y && z
    OP_OR,         // x = y || z
    OP_NOT,        // x = !y
    OP_ARRAY_REF,  // x = a[i]
    OP_ARRAY_ASSIGN, // a[i] = x
    OP_GOTO,       // goto L
    OP_IFGOTO,     // if x relop y goto L
    OP_IFNOT,      // ifnot x goto L
    OP_PARAM,      // param x
    OP_CALL,       // x = call f, n
    OP_RETURN,     // return x
    OP_FUNC,       // function f:
    OP_LABEL,      // label L:
} OpType;

typedef struct {
    OpType op;
    char result[MAX_OPERAND_LEN];    // destination
    char operand1[MAX_OPERAND_LEN];  // first source
    char operand2[MAX_OPERAND_LEN];  // second source
    char label[MAX_LABEL_LEN];       // for jumps
    int line_num;
} TACInstruction;

typedef struct {
    TACInstruction instructions[MAX_TAC_INSTRUCTIONS];
    int count;
    int temp_var_count;
    int label_count;
} TACCode;

typedef struct {
    char name[MAX_OPERAND_LEN];
    int temp_number;
} TempVar;

// TAC Code generation functions
TACCode* create_tac_code();
void emit_instruction(TACCode *tac, OpType op, const char *result, const char *op1, const char *op2);
void emit_instruction_with_label(TACCode *tac, OpType op, const char *result, const char *op1, const char *label);
void emit_goto(TACCode *tac, const char *label);
void emit_ifgoto(TACCode *tac, const char *op1, const char *relop, const char *op2, const char *label);
void emit_param(TACCode *tac, const char *operand);
void emit_call(TACCode *tac, const char *result, const char *func_name, int param_count);
void emit_return(TACCode *tac, const char *operand);
void emit_function(TACCode *tac, const char *func_name);
void emit_label(TACCode *tac, const char *label_name);

// Temporary variable management
char* new_temp_var(TACCode *tac);
char* new_label(TACCode *tac);

// Printing and output
void print_tac_code(TACCode *tac);
void print_tac_code_detailed(TACCode *tac);
void export_tac_code(TACCode *tac, const char *filename);
const char* op_to_string(OpType op);

// Cleanup
void free_tac_code(TACCode *tac);

#endif
