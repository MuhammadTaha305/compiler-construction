/*
 * MODULE 7: CODE OPTIMIZATION
 * Mini-Compiler Project - CS-346 Compiler Construction
 *
 * Implements various optimization techniques:
 * - Constant Folding
 * - Constant Propagation
 * - Common Subexpression Elimination (CSE)
 * - Dead Code Elimination
 * - Loop Optimization (Loop-Invariant Code Motion)
 */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "../common/common.h"

/* Optimization flags */
typedef struct {
    int enable_constant_folding;
    int enable_constant_propagation;
    int enable_cse;
    int enable_dead_code_elimination;
    int enable_loop_optimization;
    int verbose;
} OptimizationOptions;

/* Basic block for CFG */
typedef struct {
    int start_idx;
    int end_idx;
    int successors[10];
    int successor_count;
    int predecessors[10];
    int predecessor_count;
    int is_loop_header;
    int loop_depth;
} Block;

/* Control Flow Graph */
typedef struct {
    Block blocks[1000];
    int count;
    int *dom;           /* Dominators */
    int *loop_headers;  /* Loop header blocks */
    int loop_count;
} ControlFlowGraph;

/* Optimization statistics */
typedef struct {
    int constant_folding_count;
    int constant_propagation_count;
    int cse_count;
    int dead_code_count;
    int loop_invariant_count;
    int total_optimizations;
    double time_before_ms;
    double time_after_ms;
} OptStats;

/* Default options */
OptimizationOptions get_default_options(void);

/* Main optimization function */
TACCode* optimize_tac(TACCode *tac, OptimizationOptions opts, OptStats *stats);

/* Individual optimization passes */
int constant_folding(TACCode *tac);
int constant_propagation(TACCode *tac);
int common_subexpression_elimination(TACCode *tac);
int dead_code_elimination(TACCode *tac);
int loop_optimization(TACCode *tac, ControlFlowGraph *cfg);

/* CFG construction */
ControlFlowGraph* build_cfg(TACCode *tac);
void free_cfg(ControlFlowGraph *cfg);
void print_cfg(ControlFlowGraph *cfg);

/* Utility functions */
void print_optimization_stats(OptStats *stats);
void compare_tac(TACCode *before, TACCode *after, const char *pass_name);
TACCode* copy_tac(TACCode *tac);

#endif /* OPTIMIZER_H */
