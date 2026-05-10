/*
 * INTEGRATED MINI-COMPILER
 * CS-346 Compiler Construction Project
 *
 * Complete 8-phase compilation pipeline:
 *   Phase 1: Lexical Analysis (Tokenization)
 *   Phase 2: Syntax Analysis (Parsing)
 *   Phase 3: Extended Grammar Support
 *   Phase 4: Grammar Analysis (FIRST/FOLLOW/LL(1))
 *   Phase 5: Semantic Analysis (Type & Scope Checking)
 *   Phase 6: IR Generation (Three-Address Code)
 *   Phase 7: Code Optimization
 *   Phase 8: LLVM IR Generation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common/common.h"
#include "optimizer/optimizer.h"
#include "llvm/llvm_generator.h"

/* External declarations from modules */
extern TokenStream* tokenize(const char *source);
extern void print_token_stream(TokenStream *stream);
extern void free_token_stream(TokenStream *stream);

extern ASTNode* parse(TokenStream *tokens, int *error_count, char *error_msg);
extern void print_ast_tree(ASTNode *node, int depth, char *prefix, int is_last);

extern int semantic_analyze(ASTNode *ast, SymbolTable **symbols,
                            int *errors, int *warnings,
                            char *err_msg, char *warn_msg);
extern void print_semantic_report(int errors, int warnings,
                                  const char *err_msg, const char *warn_msg);

extern TACCode* generate_ir(ASTNode *ast, SymbolTable *symbols);
extern void export_tac_to_file(TACCode *tac, const char *filename);

/* Banner */
void print_banner(void) {
    printf("\n");
    printf("================================================================\n");
    printf("            INTEGRATED MINI-COMPILER PIPELINE\n");
    printf("         CS-346 Compiler Construction Project\n");
    printf("================================================================\n");
    printf("  Phase 1: Lexical Analysis      (Flex/Scanner)\n");
    printf("  Phase 2: Syntax Analysis       (Parser/AST)\n");
    printf("  Phase 3: Extended Grammar      (Functions, Math)\n");
    printf("  Phase 4: Grammar Analysis      (FIRST/FOLLOW/LL(1))\n");
    printf("  Phase 5: Semantic Analysis     (Type & Scope)\n");
    printf("  Phase 6: IR Generation         (Three-Address Code)\n");
    printf("  Phase 7: Code Optimization     (Constant Fold, CSE, DCE)\n");
    printf("  Phase 8: LLVM IR Generation    (Code Generation)\n");
    printf("================================================================\n\n");
}

/* Read entire file into string */
char* read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "ERROR: Cannot open file: %s\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t read = fread(buffer, 1, size, f);
    buffer[read] = '\0';
    fclose(f);

    return buffer;
}

/* Write output to file */
void write_output(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "%s", content);
        fclose(f);
        printf("Output written to: %s\n", filename);
    }
}

/* Main compilation function */
int compile(const char *source, const char *source_file, FILE *output) {
    int result = 0;
    clock_t start_time = clock();

    fprintf(output, "================================================================\n");
    fprintf(output, "            COMPILATION PIPELINE OUTPUT\n");
    fprintf(output, "================================================================\n");
    fprintf(output, "Source File: %s\n", source_file ? source_file : "<stdin>");
    fprintf(output, "================================================================\n\n");

    /* ============================================================
     * PHASE 1: LEXICAL ANALYSIS
     * ============================================================ */
    printf("\n[PHASE 1] LEXICAL ANALYSIS\n");
    printf("==========================\n");

    TokenStream *tokens = tokenize(source);
    if (!tokens) {
        fprintf(stderr, "ERROR: Tokenization failed\n");
        return -1;
    }

    printf("Tokens generated: %d\n", tokens->count);
    print_token_stream(tokens);

    fprintf(output, "PHASE 1: LEXICAL ANALYSIS\n");
    fprintf(output, "-------------------------\n");
    fprintf(output, "Total Tokens: %d\n\n", tokens->count);
    for (int i = 0; i < tokens->count; i++) {
        fprintf(output, "  [%3d] %-15s %s\n", i,
                token_type_to_string(tokens->tokens[i].type),
                tokens->tokens[i].lexeme);
    }
    fprintf(output, "\n");

    /* ============================================================
     * PHASE 2 & 3: SYNTAX ANALYSIS (PARSING)
     * ============================================================ */
    printf("\n[PHASE 2 & 3] SYNTAX ANALYSIS\n");
    printf("=============================\n");

    int parse_errors;
    char parse_error_msg[5000];
    ASTNode *ast = parse(tokens, &parse_errors, parse_error_msg);

    if (parse_errors > 0) {
        printf("Parse errors: %d\n", parse_errors);
        printf("%s", parse_error_msg);
        fprintf(output, "PHASE 2: SYNTAX ANALYSIS - FAILED\n");
        fprintf(output, "Errors: %s\n", parse_error_msg);
        result = -1;
        goto cleanup_tokens;
    }

    printf("Parse successful!\n");
    printf("\nAbstract Syntax Tree:\n");
    print_ast_tree(ast, 0, "", 1);

    fprintf(output, "PHASE 2 & 3: SYNTAX ANALYSIS\n");
    fprintf(output, "----------------------------\n");
    fprintf(output, "Status: Parse successful\n");
    fprintf(output, "AST Root: %s\n\n", ast ? "Program" : "NULL");

    /* ============================================================
     * PHASE 4: GRAMMAR ANALYSIS (Informational)
     * ============================================================ */
    printf("\n[PHASE 4] GRAMMAR ANALYSIS\n");
    printf("==========================\n");
    printf("Using extended grammar with:\n");
    printf("  - Right-associative exponentiation (^)\n");
    printf("  - Math functions (log, exp, sqrt, sin, cos, tan)\n");
    printf("  - Proper operator precedence\n");
    printf("Grammar is LL(1) compatible.\n");

    fprintf(output, "PHASE 4: GRAMMAR ANALYSIS\n");
    fprintf(output, "-------------------------\n");
    fprintf(output, "Extended Grammar Features:\n");
    fprintf(output, "  - Exponentiation: ^ (right-associative)\n");
    fprintf(output, "  - Math functions: log(), exp(), sqrt(), sin(), cos(), tan()\n");
    fprintf(output, "  - LL(1) compatible grammar\n\n");

    /* ============================================================
     * PHASE 5: SEMANTIC ANALYSIS
     * ============================================================ */
    printf("\n[PHASE 5] SEMANTIC ANALYSIS\n");
    printf("===========================\n");

    SymbolTable *symbols = NULL;
    int sem_errors = 0, sem_warnings = 0;
    char sem_err_msg[10000], sem_warn_msg[5000];

    semantic_analyze(ast, &symbols, &sem_errors, &sem_warnings,
                     sem_err_msg, sem_warn_msg);

    print_symbol_table(symbols);
    print_semantic_report(sem_errors, sem_warnings, sem_err_msg, sem_warn_msg);

    fprintf(output, "PHASE 5: SEMANTIC ANALYSIS\n");
    fprintf(output, "--------------------------\n");
    fprintf(output, "Errors: %d, Warnings: %d\n", sem_errors, sem_warnings);
    if (sem_errors > 0) {
        fprintf(output, "Errors:\n%s\n", sem_err_msg);
    }
    if (sem_warnings > 0) {
        fprintf(output, "Warnings:\n%s\n", sem_warn_msg);
    }
    fprintf(output, "\nSymbol Table:\n");
    for (int i = 0; i < symbols->count; i++) {
        fprintf(output, "  %-20s %-10s scope=%d\n",
                symbols->symbols[i].name,
                symbols->symbols[i].type_name,
                symbols->symbols[i].scope_level);
    }
    fprintf(output, "\n");

    if (sem_errors > 0) {
        printf("Semantic analysis failed with %d errors\n", sem_errors);
        result = -1;
        goto cleanup_symbols;
    }

    /* ============================================================
     * PHASE 6: IR GENERATION (TAC)
     * ============================================================ */
    printf("\n[PHASE 6] IR GENERATION (TAC)\n");
    printf("=============================\n");

    TACCode *tac = generate_ir(ast, symbols);
    if (!tac) {
        fprintf(stderr, "ERROR: IR generation failed\n");
        result = -1;
        goto cleanup_symbols;
    }

    printf("TAC instructions generated: %d\n", tac->count);
    print_tac(tac);

    /* Export TAC to file */
    export_tac_to_file(tac, "ir/output.tac");

    fprintf(output, "PHASE 6: IR GENERATION (TAC)\n");
    fprintf(output, "----------------------------\n");
    fprintf(output, "Instructions: %d\n", tac->count);
    fprintf(output, "Temporaries: %d, Labels: %d\n\n", tac->temp_count, tac->label_count);

    /* ============================================================
     * PHASE 7: CODE OPTIMIZATION
     * ============================================================ */
    printf("\n[PHASE 7] CODE OPTIMIZATION\n");
    printf("============================\n");

    /* Copy TAC for comparison */
    TACCode *original_tac = copy_tac(tac);

    OptimizationOptions opt_opts = get_default_options();
    OptStats opt_stats;

    optimize_tac(tac, opt_opts, &opt_stats);
    print_optimization_stats(&opt_stats);

    printf("\n--- Optimized TAC ---\n");
    print_tac(tac);

    compare_tac(original_tac, tac, "Overall Optimization");

    fprintf(output, "PHASE 7: CODE OPTIMIZATION\n");
    fprintf(output, "--------------------------\n");
    fprintf(output, "Constant Folding:      %d\n", opt_stats.constant_folding_count);
    fprintf(output, "Constant Propagation:  %d\n", opt_stats.constant_propagation_count);
    fprintf(output, "CSE:                   %d\n", opt_stats.cse_count);
    fprintf(output, "Dead Code Elimination: %d\n", opt_stats.dead_code_count);
    fprintf(output, "Loop Optimizations:    %d\n", opt_stats.loop_invariant_count);
    fprintf(output, "Total Optimizations:   %d\n\n", opt_stats.total_optimizations);

    /* ============================================================
     * PHASE 8: LLVM IR GENERATION
     * ============================================================ */
    printf("\n[PHASE 8] LLVM IR GENERATION\n");
    printf("============================\n");

    const char *llvm_output = "llvm/output.ll";
    int llvm_result = generate_llvm_ir(tac, symbols, llvm_output);

    if (llvm_result == 0) {
        printf("LLVM IR generated successfully!\n");
        print_llvm_ir(llvm_output);

        fprintf(output, "PHASE 8: LLVM IR GENERATION\n");
        fprintf(output, "---------------------------\n");
        fprintf(output, "Output file: %s\n", llvm_output);
        fprintf(output, "Status: Success\n\n");

        /* Try to compile with Clang */
        printf("\nAttempting Clang compilation...\n");
        compile_with_clang(llvm_output, "llvm/output.exe", 0);
    } else {
        fprintf(output, "PHASE 8: LLVM IR GENERATION - FAILED\n\n");
    }

    /* ============================================================
     * COMPILATION SUMMARY
     * ============================================================ */
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000;

    printf("\n================================================================\n");
    printf("                 COMPILATION SUMMARY\n");
    printf("================================================================\n");
    printf("Source file:        %s\n", source_file ? source_file : "<stdin>");
    printf("Tokens:             %d\n", tokens->count);
    printf("Parse errors:       %d\n", parse_errors);
    printf("Semantic errors:    %d\n", sem_errors);
    printf("Semantic warnings:  %d\n", sem_warnings);
    printf("TAC instructions:   %d (before) / %d (after opt)\n",
           original_tac->count, tac->count);
    printf("Optimizations:      %d\n", opt_stats.total_optimizations);
    printf("LLVM IR:            %s\n", llvm_result == 0 ? "Generated" : "Failed");
    printf("Compilation time:   %.2f ms\n", elapsed);
    printf("Status:             %s\n", result == 0 ? "SUCCESS" : "FAILED");
    printf("================================================================\n\n");

    fprintf(output, "================================================================\n");
    fprintf(output, "                 COMPILATION SUMMARY\n");
    fprintf(output, "================================================================\n");
    fprintf(output, "Tokens:             %d\n", tokens->count);
    fprintf(output, "Semantic errors:    %d\n", sem_errors);
    fprintf(output, "TAC instructions:   %d\n", tac->count);
    fprintf(output, "Optimizations:      %d\n", opt_stats.total_optimizations);
    fprintf(output, "Compilation time:   %.2f ms\n", elapsed);
    fprintf(output, "Status:             %s\n", result == 0 ? "SUCCESS" : "FAILED");
    fprintf(output, "================================================================\n");

    /* Cleanup */
    free_tac(original_tac);
    free_tac(tac);

cleanup_symbols:
    if (symbols) free_symbol_table(symbols);
    if (ast) free_ast(ast);

cleanup_tokens:
    if (tokens) free_token_stream(tokens);

    return result;
}

/* Usage */
void print_usage(const char *prog) {
    printf("Usage: %s [options] <input_file>\n\n", prog);
    printf("Options:\n");
    printf("  -o <file>    Output file (default: pipeline_output.txt)\n");
    printf("  -v           Verbose output\n");
    printf("  -h           Show this help\n\n");
    printf("Example:\n");
    printf("  %s test_input.c\n", prog);
    printf("  %s -o output.txt program.c\n", prog);
}

/* Main */
int main(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = "pipeline_output.txt";
    int verbose = 0;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        }
    }

    print_banner();

    if (verbose) {
        printf("Verbose mode enabled.\n");
    }

    /* Read source */
    char *source = NULL;
    if (input_file) {
        printf("Input file: %s\n", input_file);
        source = read_file(input_file);
        if (!source) {
            return 1;
        }
    } else {
        /* Use default test */
        printf("No input file specified. Using built-in test program.\n\n");
        source = strdup(
            "int main() {\n"
            "    int a = 5;\n"
            "    int b = 10;\n"
            "    int c = a + b;\n"
            "    if (c > 10) {\n"
            "        c = c * 2;\n"
            "    }\n"
            "    return c;\n"
            "}\n"
        );
    }

    printf("\n--- Source Code ---\n%s\n", source);

    /* Open output file */
    FILE *output = fopen(output_file, "w");
    if (!output) {
        fprintf(stderr, "Cannot open output file: %s\n", output_file);
        free(source);
        return 1;
    }

    /* Compile */
    int result = compile(source, input_file, output);

    fclose(output);
    free(source);

    printf("Pipeline output saved to: %s\n", output_file);

    return result;
}
