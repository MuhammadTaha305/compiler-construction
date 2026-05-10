/*
 * MODULE 8: LLVM IR GENERATION
 * Mini-Compiler Project - CS-346 Compiler Construction
 *
 * Generates LLVM IR from TAC for compilation with Clang.
 */

#ifndef LLVM_GENERATOR_H
#define LLVM_GENERATOR_H

#include "../common/common.h"

/* LLVM generation options */
typedef struct {
    int optimize;           /* Apply LLVM optimizations */
    int debug_info;         /* Include debug information */
    int verbose;            /* Verbose output */
    char output_file[256];  /* Output .ll file */
} LLVMOptions;

/* Generate LLVM IR from TAC */
int generate_llvm_ir(TACCode *tac, SymbolTable *symbols, const char *output_file);

/* Generate LLVM IR with options */
int generate_llvm_ir_with_options(TACCode *tac, SymbolTable *symbols, LLVMOptions *opts);

/* Compile LLVM IR with Clang */
int compile_with_clang(const char *ll_file, const char *output_exe, int optimize);

/* Utility functions */
const char* get_llvm_type(DataType type);
void print_llvm_ir(const char *filename);

#endif /* LLVM_GENERATOR_H */
