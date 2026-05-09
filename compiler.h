/*
 * INTEGRATED COMPILER PIPELINE
 * ======================
 * 
 * Full Compilation Flow:
 * Source Code → [Lexer] → [Parser] → [Semantic Analysis] → [TAC Generation] → Output
 * 
 * Modules:
 * 1. Lexical Analysis (Flex)
 * 2. Syntax Analysis (Bison)
 * 3. Extended Grammar Support
 * 4. FIRST & Follow Sets / LL(1) Validation
 * 5. Semantic Analysis (Type & Scope Checking)
 * 6. Intermediate Representation (TAC)
 */

#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations */
typedef struct {
    int temp_var_count;
    int label_count;
} CompilationContext;

typedef struct {
    char name[100];
    char type[50];
    int scope;
} Variable;

typedef struct {
    Variable vars[1000];
    int count;
} VariableTable;

typedef struct {
    char instruction[500];
    int line_num;
} TACInstruction;

typedef struct {
    TACInstruction instructions[10000];
    int count;
    int temp_count;
    int label_count;
} TACProgram;

typedef struct {
    int errors;
    int warnings;
    char messages[5000];
} CompilationReport;

/* Compiler stages */
typedef struct {
    FILE *input_file;
    FILE *output_file;
    VariableTable symbol_table;
    TACProgram tac;
    CompilationReport report;
    CompilationContext context;
} Compiler;

/* Function prototypes */
Compiler* compiler_init(const char *input_filename, const char *output_filename);
int compiler_run(Compiler *compiler);
void compiler_cleanup(Compiler *compiler);
void compiler_report(Compiler *compiler);

#endif
