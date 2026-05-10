/*
 * MODULE 7: CODE OPTIMIZATION (Implementation)
 * Mini-Compiler Project - CS-346 Compiler Construction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "optimizer.h"

/* ============================================================
 * UTILITY FUNCTIONS
 * ============================================================ */

/* Check if string is a number */
static int is_number(const char *s) {
    if (!s || !*s) return 0;
    if (*s == '-') s++;
    while (*s) {
        if (!isdigit(*s) && *s != '.') return 0;
        s++;
    }
    return 1;
}

/* Check if operand is a constant */
static int is_constant(const char *op) {
    return is_number(op);
}

/* Check if operand is a temporary */
static int is_temp(const char *op) {
    return op && op[0] == 't' && isdigit(op[1]);
}

/* Get default optimization options */
OptimizationOptions get_default_options(void) {
    OptimizationOptions opts;
    opts.enable_constant_folding = 1;
    opts.enable_constant_propagation = 1;
    opts.enable_cse = 1;
    opts.enable_dead_code_elimination = 1;
    opts.enable_loop_optimization = 1;
    opts.verbose = 1;
    return opts;
}

/* Copy TAC code */
TACCode* copy_tac(TACCode *src) {
    TACCode *dst = (TACCode *)malloc(sizeof(TACCode));
    memcpy(dst, src, sizeof(TACCode));
    return dst;
}

/* ============================================================
 * CONSTANT FOLDING
 * Evaluate constant expressions at compile time
 * ============================================================ */
int constant_folding(TACCode *tac) {
    int changes = 0;

    printf("\n  [Constant Folding]\n");

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        /* Skip dead code */
        if (instr->is_dead) continue;

        /* Check for binary operations with constant operands */
        if ((instr->op >= OP_ADD && instr->op <= OP_MOD) ||
            (instr->op >= OP_LT && instr->op <= OP_NE)) {

            if (is_constant(instr->arg1) && is_constant(instr->arg2)) {
                double v1 = atof(instr->arg1);
                double v2 = atof(instr->arg2);
                double result = 0;

                switch (instr->op) {
                    case OP_ADD: result = v1 + v2; break;
                    case OP_SUB: result = v1 - v2; break;
                    case OP_MUL: result = v1 * v2; break;
                    case OP_DIV: result = (v2 != 0) ? v1 / v2 : 0; break;
                    case OP_MOD: result = (int)v1 % (int)v2; break;
                    case OP_LT:  result = v1 < v2; break;
                    case OP_LE:  result = v1 <= v2; break;
                    case OP_GT:  result = v1 > v2; break;
                    case OP_GE:  result = v1 >= v2; break;
                    case OP_EQ:  result = v1 == v2; break;
                    case OP_NE:  result = v1 != v2; break;
                    default: continue;
                }

                printf("    %s = %s %s %s  =>  %s = %.0f\n",
                       instr->result, instr->arg1,
                       tac_op_to_string(instr->op), instr->arg2,
                       instr->result, result);

                /* Convert to assignment */
                instr->op = OP_ASSIGN;
                snprintf(instr->arg1, MAX_OPERAND_LEN, "%.0f", result);
                instr->arg2[0] = '\0';
                changes++;
            }
        }

        /* Unary minus with constant */
        if (instr->op == OP_UMINUS && is_constant(instr->arg1)) {
            double v = -atof(instr->arg1);
            printf("    %s = -%s  =>  %s = %.0f\n",
                   instr->result, instr->arg1, instr->result, v);

            instr->op = OP_ASSIGN;
            snprintf(instr->arg1, MAX_OPERAND_LEN, "%.0f", v);
            changes++;
        }
    }

    printf("    Folded %d expressions\n", changes);
    return changes;
}

/* ============================================================
 * CONSTANT PROPAGATION
 * Replace variables with their constant values
 * ============================================================ */

typedef struct {
    char var[MAX_OPERAND_LEN];
    char value[MAX_OPERAND_LEN];
    int valid;
} ConstantValue;

int constant_propagation(TACCode *tac) {
    int changes = 0;
    ConstantValue constants[500];
    int const_count = 0;

    printf("\n  [Constant Propagation]\n");

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        if (instr->is_dead) continue;

        /* Clear constant for redefined variables */
        if (instr->result[0]) {
            for (int j = 0; j < const_count; j++) {
                if (strcmp(constants[j].var, instr->result) == 0) {
                    constants[j].valid = 0;
                }
            }
        }

        /* Track constant assignments */
        if (instr->op == OP_ASSIGN && is_constant(instr->arg1)) {
            int found = 0;
            for (int j = 0; j < const_count; j++) {
                if (strcmp(constants[j].var, instr->result) == 0) {
                    strcpy(constants[j].value, instr->arg1);
                    constants[j].valid = 1;
                    found = 1;
                    break;
                }
            }
            if (!found && const_count < 500) {
                strcpy(constants[const_count].var, instr->result);
                strcpy(constants[const_count].value, instr->arg1);
                constants[const_count].valid = 1;
                const_count++;
            }
        }

        /* Replace uses of constants */
        for (int j = 0; j < const_count; j++) {
            if (!constants[j].valid) continue;

            if (strcmp(instr->arg1, constants[j].var) == 0 &&
                !is_constant(instr->arg1)) {
                printf("    Replacing %s with %s in instruction %d\n",
                       instr->arg1, constants[j].value, i);
                strcpy(instr->arg1, constants[j].value);
                changes++;
            }

            if (strcmp(instr->arg2, constants[j].var) == 0 &&
                !is_constant(instr->arg2)) {
                printf("    Replacing %s with %s in instruction %d\n",
                       instr->arg2, constants[j].value, i);
                strcpy(instr->arg2, constants[j].value);
                changes++;
            }
        }

        /* Invalidate constants at labels and function calls */
        if (instr->op == OP_LABEL || instr->op == OP_FUNC ||
            instr->op == OP_CALL || instr->op == OP_GOTO) {
            for (int j = 0; j < const_count; j++) {
                constants[j].valid = 0;
            }
        }
    }

    printf("    Propagated %d constants\n", changes);
    return changes;
}

/* ============================================================
 * COMMON SUBEXPRESSION ELIMINATION (CSE)
 * Reuse previously computed expressions
 * ============================================================ */

typedef struct {
    TACOpType op;
    char arg1[MAX_OPERAND_LEN];
    char arg2[MAX_OPERAND_LEN];
    char result[MAX_OPERAND_LEN];
    int valid;
} Expression;

int common_subexpression_elimination(TACCode *tac) {
    int changes = 0;
    Expression exprs[500];
    int expr_count = 0;

    printf("\n  [Common Subexpression Elimination]\n");

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        if (instr->is_dead) continue;

        /* Invalidate expressions using redefined variable */
        if (instr->result[0]) {
            for (int j = 0; j < expr_count; j++) {
                if (strcmp(exprs[j].arg1, instr->result) == 0 ||
                    strcmp(exprs[j].arg2, instr->result) == 0 ||
                    strcmp(exprs[j].result, instr->result) == 0) {
                    exprs[j].valid = 0;
                }
            }
        }

        /* Check for computable expressions */
        if (instr->op >= OP_ADD && instr->op <= OP_OR &&
            instr->arg1[0] && instr->arg2[0]) {

            /* Look for existing computation */
            int found = -1;
            for (int j = 0; j < expr_count; j++) {
                if (!exprs[j].valid) continue;

                if (exprs[j].op == instr->op &&
                    strcmp(exprs[j].arg1, instr->arg1) == 0 &&
                    strcmp(exprs[j].arg2, instr->arg2) == 0) {
                    found = j;
                    break;
                }

                /* Check for commutative operations */
                if ((instr->op == OP_ADD || instr->op == OP_MUL ||
                     instr->op == OP_AND || instr->op == OP_OR) &&
                    exprs[j].op == instr->op &&
                    strcmp(exprs[j].arg1, instr->arg2) == 0 &&
                    strcmp(exprs[j].arg2, instr->arg1) == 0) {
                    found = j;
                    break;
                }
            }

            if (found >= 0) {
                /* Replace with copy */
                printf("    %s = %s %s %s  =>  %s = %s (reuse)\n",
                       instr->result, instr->arg1,
                       tac_op_to_string(instr->op), instr->arg2,
                       instr->result, exprs[found].result);

                instr->op = OP_COPY;
                strcpy(instr->arg1, exprs[found].result);
                instr->arg2[0] = '\0';
                changes++;
            } else {
                /* Record new expression */
                if (expr_count < 500) {
                    exprs[expr_count].op = instr->op;
                    strcpy(exprs[expr_count].arg1, instr->arg1);
                    strcpy(exprs[expr_count].arg2, instr->arg2);
                    strcpy(exprs[expr_count].result, instr->result);
                    exprs[expr_count].valid = 1;
                    expr_count++;
                }
            }
        }

        /* Invalidate on control flow */
        if (instr->op == OP_LABEL || instr->op == OP_FUNC ||
            instr->op == OP_GOTO || instr->op == OP_IFGOTO ||
            instr->op == OP_IFFALSE || instr->op == OP_CALL) {
            for (int j = 0; j < expr_count; j++) {
                exprs[j].valid = 0;
            }
        }
    }

    printf("    Eliminated %d redundant computations\n", changes);
    return changes;
}

/* ============================================================
 * DEAD CODE ELIMINATION
 * Remove code whose results are never used
 * ============================================================ */
int dead_code_elimination(TACCode *tac) {
    int changes = 0;
    int used[MAX_TAC_INSTRUCTIONS];

    printf("\n  [Dead Code Elimination]\n");

    /* Mark all instructions as potentially dead */
    for (int i = 0; i < tac->count; i++) {
        used[i] = 0;
    }

    /* Mark essential instructions */
    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        /* Essential operations (side effects) */
        if (instr->op == OP_CALL || instr->op == OP_RETURN ||
            instr->op == OP_PARAM || instr->op == OP_FUNC ||
            instr->op == OP_LABEL || instr->op == OP_GOTO ||
            instr->op == OP_IFGOTO || instr->op == OP_IFFALSE ||
            instr->op == OP_ARRAY_ASSIGN) {
            used[i] = 1;
        }

        /* Non-temp assignments are essential (visible outside) */
        if ((instr->op == OP_ASSIGN || instr->op == OP_COPY) &&
            !is_temp(instr->result)) {
            used[i] = 1;
        }
    }

    /* Propagate liveness backwards */
    int changed;
    do {
        changed = 0;
        for (int i = tac->count - 1; i >= 0; i--) {
            if (!used[i]) continue;

            TACInstruction *instr = &tac->instructions[i];

            /* Find definitions of used operands */
            for (int j = i - 1; j >= 0; j--) {
                TACInstruction *def = &tac->instructions[j];

                if (!used[j]) {
                    if ((strcmp(def->result, instr->arg1) == 0 ||
                         strcmp(def->result, instr->arg2) == 0) &&
                        def->result[0]) {
                        used[j] = 1;
                        changed = 1;
                    }
                }
            }
        }
    } while (changed);

    /* Mark dead instructions */
    for (int i = 0; i < tac->count; i++) {
        if (!used[i] && !tac->instructions[i].is_dead) {
            printf("    Dead: %d: %s = %s %s %s\n", i,
                   tac->instructions[i].result,
                   tac->instructions[i].arg1,
                   tac_op_to_string(tac->instructions[i].op),
                   tac->instructions[i].arg2);
            tac->instructions[i].is_dead = 1;
            changes++;
        }
    }

    printf("    Eliminated %d dead instructions\n", changes);
    return changes;
}

/* ============================================================
 * CONTROL FLOW GRAPH
 * ============================================================ */
ControlFlowGraph* build_cfg(TACCode *tac) {
    ControlFlowGraph *cfg = (ControlFlowGraph *)calloc(1, sizeof(ControlFlowGraph));

    /* Identify leaders (start of basic blocks) */
    int is_leader[MAX_TAC_INSTRUCTIONS] = {0};
    is_leader[0] = 1;

    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        /* After a jump, next instruction is a leader */
        if (instr->op == OP_GOTO || instr->op == OP_IFGOTO ||
            instr->op == OP_IFFALSE || instr->op == OP_RETURN) {
            if (i + 1 < tac->count) {
                is_leader[i + 1] = 1;
            }
        }

        /* Labels are leaders */
        if (instr->op == OP_LABEL || instr->op == OP_FUNC) {
            is_leader[i] = 1;
        }
    }

    /* Create basic blocks */
    int block_start = 0;
    for (int i = 0; i <= tac->count; i++) {
        if (i == tac->count || (is_leader[i] && i > block_start)) {
            Block *b = &cfg->blocks[cfg->count++];
            b->start_idx = block_start;
            b->end_idx = i - 1;
            b->successor_count = 0;
            b->predecessor_count = 0;
            b->is_loop_header = 0;
            b->loop_depth = 0;

            /* Mark instructions with block ID */
            for (int j = block_start; j < i; j++) {
                tac->instructions[j].block_id = cfg->count - 1;
            }

            block_start = i;
        }
    }

    printf("\n  Built CFG: %d basic blocks\n", cfg->count);
    return cfg;
}

void free_cfg(ControlFlowGraph *cfg) {
    if (cfg) {
        if (cfg->dom) free(cfg->dom);
        if (cfg->loop_headers) free(cfg->loop_headers);
        free(cfg);
    }
}

void print_cfg(ControlFlowGraph *cfg) {
    printf("\n  === Control Flow Graph ===\n");
    for (int i = 0; i < cfg->count; i++) {
        Block *b = &cfg->blocks[i];
        printf("  Block %d: instructions %d-%d", i, b->start_idx, b->end_idx);
        if (b->is_loop_header) printf(" [LOOP HEADER]");
        printf("\n");
    }
}

/* ============================================================
 * LOOP OPTIMIZATION
 * Move loop-invariant code out of loops
 * ============================================================ */
int loop_optimization(TACCode *tac, ControlFlowGraph *cfg) {
    int changes = 0;

    printf("\n  [Loop Optimization]\n");

    /* Simplified: look for while loop patterns */
    for (int i = 0; i < tac->count; i++) {
        TACInstruction *instr = &tac->instructions[i];

        if (instr->op == OP_LABEL) {
            /* Find backward jumps to this label (loop headers) */
            for (int j = i + 1; j < tac->count; j++) {
                TACInstruction *jump = &tac->instructions[j];
                if ((jump->op == OP_GOTO ||
                     jump->op == OP_IFGOTO ||
                     jump->op == OP_IFFALSE) &&
                    strcmp(jump->label[0] ? jump->label : jump->arg1,
                           instr->arg1) == 0) {

                    printf("    Found loop: %d to %d\n", i, j);

                    /* Look for invariant computations */
                    for (int k = i + 1; k < j; k++) {
                        TACInstruction *loop_instr = &tac->instructions[k];

                        if (loop_instr->is_dead) continue;

                        /* Check if both operands are defined outside loop */
                        if (loop_instr->op >= OP_ADD && loop_instr->op <= OP_OR) {
                            int arg1_invariant = is_constant(loop_instr->arg1);
                            int arg2_invariant = is_constant(loop_instr->arg2);

                            /* Check if args are not modified in loop */
                            for (int m = i + 1; m < j && (arg1_invariant || arg2_invariant); m++) {
                                TACInstruction *mod = &tac->instructions[m];
                                if (strcmp(mod->result, loop_instr->arg1) == 0)
                                    arg1_invariant = 0;
                                if (strcmp(mod->result, loop_instr->arg2) == 0)
                                    arg2_invariant = 0;
                            }

                            if (arg1_invariant && arg2_invariant) {
                                printf("    Loop invariant at %d: %s = %s %s %s\n",
                                       k, loop_instr->result, loop_instr->arg1,
                                       tac_op_to_string(loop_instr->op),
                                       loop_instr->arg2);
                                changes++;
                                /* Note: actual code motion would require restructuring */
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    printf("    Found %d loop-invariant expressions\n", changes);
    return changes;
}

/* ============================================================
 * MAIN OPTIMIZATION FUNCTION
 * ============================================================ */
TACCode* optimize_tac(TACCode *tac, OptimizationOptions opts, OptStats *stats) {
    printf("\n========== CODE OPTIMIZATION ==========\n");

    /* Initialize stats */
    memset(stats, 0, sizeof(OptStats));

    /* Build CFG */
    ControlFlowGraph *cfg = build_cfg(tac);

    /* Apply optimizations iteratively */
    int total_changes;
    int iterations = 0;

    do {
        total_changes = 0;

        if (opts.enable_constant_folding) {
            int c = constant_folding(tac);
            stats->constant_folding_count += c;
            total_changes += c;
        }

        if (opts.enable_constant_propagation) {
            int c = constant_propagation(tac);
            stats->constant_propagation_count += c;
            total_changes += c;
        }

        if (opts.enable_cse) {
            int c = common_subexpression_elimination(tac);
            stats->cse_count += c;
            total_changes += c;
        }

        if (opts.enable_dead_code_elimination) {
            int c = dead_code_elimination(tac);
            stats->dead_code_count += c;
            total_changes += c;
        }

        iterations++;
    } while (total_changes > 0 && iterations < 10);

    /* Loop optimization (single pass) */
    if (opts.enable_loop_optimization) {
        int c = loop_optimization(tac, cfg);
        stats->loop_invariant_count = c;
    }

    stats->total_optimizations =
        stats->constant_folding_count +
        stats->constant_propagation_count +
        stats->cse_count +
        stats->dead_code_count +
        stats->loop_invariant_count;

    free_cfg(cfg);

    printf("\n  Optimization complete after %d iterations\n", iterations);
    printf("========================================\n");

    return tac;
}

/* ============================================================
 * STATISTICS AND COMPARISON
 * ============================================================ */
void print_optimization_stats(OptStats *stats) {
    printf("\n========== OPTIMIZATION STATISTICS ==========\n");
    printf("Constant Folding:      %d\n", stats->constant_folding_count);
    printf("Constant Propagation:  %d\n", stats->constant_propagation_count);
    printf("CSE:                   %d\n", stats->cse_count);
    printf("Dead Code Elimination: %d\n", stats->dead_code_count);
    printf("Loop Invariants:       %d\n", stats->loop_invariant_count);
    printf("---------------------------------------------\n");
    printf("Total Optimizations:   %d\n", stats->total_optimizations);
    printf("=============================================\n");
}

void compare_tac(TACCode *before, TACCode *after, const char *pass_name) {
    int before_count = 0, after_count = 0;

    for (int i = 0; i < before->count; i++) {
        if (!before->instructions[i].is_dead) before_count++;
    }
    for (int i = 0; i < after->count; i++) {
        if (!after->instructions[i].is_dead) after_count++;
    }

    printf("\n--- %s ---\n", pass_name);
    printf("Before: %d instructions\n", before_count);
    printf("After:  %d instructions\n", after_count);
    printf("Reduction: %d (%.1f%%)\n",
           before_count - after_count,
           before_count > 0 ? 100.0 * (before_count - after_count) / before_count : 0);
}

/* Standalone main for testing */
#ifdef OPTIMIZER_STANDALONE
int main(int argc, char *argv[]) {
    printf("=== MODULE 7: CODE OPTIMIZATION ===\n\n");

    /* Create sample TAC for testing */
    TACCode *tac = create_tac();

    /* Sample: x = 2 + 3; y = x * 4; z = 2 + 3; */
    emit_tac(tac, OP_ADD, "t0", "2", "3");      /* t0 = 2 + 3 */
    emit_tac(tac, OP_ASSIGN, "x", "t0", NULL);  /* x = t0 */
    emit_tac(tac, OP_MUL, "t1", "x", "4");      /* t1 = x * 4 */
    emit_tac(tac, OP_ASSIGN, "y", "t1", NULL);  /* y = t1 */
    emit_tac(tac, OP_ADD, "t2", "2", "3");      /* t2 = 2 + 3 (redundant) */
    emit_tac(tac, OP_ASSIGN, "z", "t2", NULL);  /* z = t2 */
    emit_tac(tac, OP_ADD, "t3", "a", "b");      /* t3 = a + b (dead) */

    printf("--- Before Optimization ---\n");
    print_tac(tac);

    /* Copy for comparison */
    TACCode *before = copy_tac(tac);

    /* Run optimization */
    OptimizationOptions opts = get_default_options();
    OptStats stats;

    optimize_tac(tac, opts, &stats);

    printf("\n--- After Optimization ---\n");
    print_tac(tac);

    print_optimization_stats(&stats);
    compare_tac(before, tac, "Overall");

    free_tac(before);
    free_tac(tac);

    return 0;
}
#endif
