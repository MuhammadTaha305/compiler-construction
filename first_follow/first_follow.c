#include "first_follow.h"
#include <ctype.h>

FirstSets* compute_first_sets(Grammar *g) {
    FirstSets *fs = (FirstSets *)malloc(sizeof(FirstSets));
    fs->count = g->symbols.num_nonterminals;

    // Initialize FIRST sets for each nonterminal
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        fs->first[i].count = 0;
    }

    // Add terminals to FIRST(terminals)
    // Note: we don't store FIRST for terminals, only for nonterminals

    int changed = 1;
    int iterations = 0;

    while (changed && iterations < 100) {
        changed = 0;
        iterations++;

        // For each production A -> X1 X2 ... Xn
        for (int p = 0; p < g->num_productions; p++) {
            Production *prod = &g->productions[p];

            // Find index of LHS nonterminal
            int lhs_idx = -1;
            for (int i = 0; i < g->symbols.num_nonterminals; i++) {
                if (strcmp(g->symbols.nonterminals[i], prod->lhs) == 0) {
                    lhs_idx = i;
                    break;
                }
            }

            if (lhs_idx == -1) continue;

            // Handle epsilon productions
            if (prod->is_epsilon) {
                if (!set_contains(&fs->first[lhs_idx], "epsilon")) {
                    set_add(&fs->first[lhs_idx], "epsilon");
                    changed = 1;
                }
                continue;
            }

            // Process RHS: A -> X1 X2 ... Xn
            for (int i = 0; i < prod->rhs_count; i++) {
                char *symbol = prod->rhs[i];
                int old_count = fs->first[lhs_idx].count;

                if (is_terminal(g, symbol)) {
                    // Terminal: add to FIRST(A), then stop (terminals not nullable)
                    set_add(&fs->first[lhs_idx], symbol);
                    if (fs->first[lhs_idx].count > old_count) {
                        changed = 1;
                    }
                    break;  // Terminals are not nullable, so stop processing
                } else if (is_nonterminal(g, symbol)) {
                    // Nonterminal: add FIRST(Xi) - {ε} to FIRST(A)
                    int sym_idx = -1;
                    for (int j = 0; j < g->symbols.num_nonterminals; j++) {
                        if (strcmp(g->symbols.nonterminals[j], symbol) == 0) {
                            sym_idx = j;
                            break;
                        }
                    }

                    if (sym_idx != -1) {
                        for (int j = 0; j < fs->first[sym_idx].count; j++) {
                            if (strcmp(fs->first[sym_idx].symbols[j], "epsilon") != 0) {
                                set_add(&fs->first[lhs_idx], fs->first[sym_idx].symbols[j]);
                            }
                        }
                    }

                    // If Xi is not nullable, stop
                    int xi_nullable = 0;
                    if (sym_idx != -1) {
                        for (int j = 0; j < fs->first[sym_idx].count; j++) {
                            if (strcmp(fs->first[sym_idx].symbols[j], "epsilon") == 0) {
                                xi_nullable = 1;
                                break;
                            }
                        }
                    }

                    if (fs->first[lhs_idx].count > old_count) {
                        changed = 1;
                    }

                    if (!xi_nullable) break;
                }
            }

            // If all RHS are nullable (ε productions), add ε to FIRST(A)
            int all_nullable = 1;
            for (int i = 0; i < prod->rhs_count; i++) {
                char *symbol = prod->rhs[i];
                int nullable = 0;

                if (is_terminal(g, symbol)) {
                    nullable = 0;
                } else if (is_nonterminal(g, symbol)) {
                    int sym_idx = -1;
                    for (int j = 0; j < g->symbols.num_nonterminals; j++) {
                        if (strcmp(g->symbols.nonterminals[j], symbol) == 0) {
                            sym_idx = j;
                            break;
                        }
                    }
                    if (sym_idx != -1) {
                        for (int j = 0; j < fs->first[sym_idx].count; j++) {
                            if (strcmp(fs->first[sym_idx].symbols[j], "epsilon") == 0) {
                                nullable = 1;
                                break;
                            }
                        }
                    }
                }

                if (!nullable) {
                    all_nullable = 0;
                    break;
                }
            }

            if (all_nullable && prod->rhs_count > 0) {
                if (!set_contains(&fs->first[lhs_idx], "epsilon")) {
                    set_add(&fs->first[lhs_idx], "epsilon");
                    changed = 1;
                }
            }
        }
    }

    return fs;
}

FollowSets* compute_follow_sets(Grammar *g, FirstSets *fs) {
    FollowSets *fos = (FollowSets *)malloc(sizeof(FollowSets));
    fos->count = g->symbols.num_nonterminals;

    // Initialize FOLLOW sets
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        fos->follow[i].count = 0;
    }

    // Add $ to FOLLOW(start symbol)
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        if (strcmp(g->symbols.nonterminals[i], g->start_symbol) == 0) {
            set_add(&fos->follow[i], "$");
            break;
        }
    }

    int changed = 1;
    int iterations = 0;

    while (changed && iterations < 100) {
        changed = 0;
        iterations++;

        // For each production A -> X1 X2 ... Xn
        for (int p = 0; p < g->num_productions; p++) {
            Production *prod = &g->productions[p];

            // Process each symbol in RHS
            for (int i = 0; i < prod->rhs_count; i++) {
                char *B = prod->rhs[i];

                if (!is_nonterminal(g, B)) continue;

                // Find index of B
                int b_idx = -1;
                for (int j = 0; j < g->symbols.num_nonterminals; j++) {
                    if (strcmp(g->symbols.nonterminals[j], B) == 0) {
                        b_idx = j;
                        break;
                    }
                }

                if (b_idx == -1) continue;

                // Get FIRST of remaining symbols (Xi+1, Xi+2, ..., Xn)
                SymbolSet first_rest;
                first_rest.count = 0;

                int all_rest_nullable = 1;
                for (int j = i + 1; j < prod->rhs_count; j++) {
                    char *C = prod->rhs[j];

                    if (is_terminal(g, C)) {
                        set_add(&first_rest, C);
                        all_rest_nullable = 0;
                    } else if (is_nonterminal(g, C)) {
                        int c_idx = -1;
                        for (int k = 0; k < g->symbols.num_nonterminals; k++) {
                            if (strcmp(g->symbols.nonterminals[k], C) == 0) {
                                c_idx = k;
                                break;
                            }
                        }

                        if (c_idx != -1) {
                            int c_nullable = 0;
                            for (int k = 0; k < fs->first[c_idx].count; k++) {
                                if (strcmp(fs->first[c_idx].symbols[k], "epsilon") == 0) {
                                    c_nullable = 1;
                                } else {
                                    set_add(&first_rest, fs->first[c_idx].symbols[k]);
                                }
                            }
                            if (!c_nullable) {
                                all_rest_nullable = 0;
                            }
                        }
                    }
                }

                // Add FIRST(rest) to FOLLOW(B)
                int old_count = fos->follow[b_idx].count;
                for (int j = 0; j < first_rest.count; j++) {
                    set_add(&fos->follow[b_idx], first_rest.symbols[j]);
                }

                // If all rest are nullable, add FOLLOW(A) to FOLLOW(B)
                if (all_rest_nullable || prod->rhs_count == i + 1) {
                    int a_idx = -1;
                    for (int j = 0; j < g->symbols.num_nonterminals; j++) {
                        if (strcmp(g->symbols.nonterminals[j], prod->lhs) == 0) {
                            a_idx = j;
                            break;
                        }
                    }

                    if (a_idx != -1) {
                        for (int j = 0; j < fos->follow[a_idx].count; j++) {
                            set_add(&fos->follow[b_idx], fos->follow[a_idx].symbols[j]);
                        }
                    }
                }

                if (fos->follow[b_idx].count > old_count) {
                    changed = 1;
                }
            }
        }
    }

    return fos;
}

int is_nullable(Grammar *g, FirstSets *fs, const char *symbol) {
    if (is_terminal(g, symbol)) return 0;

    int idx = -1;
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        if (strcmp(g->symbols.nonterminals[i], symbol) == 0) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return 0;

    for (int i = 0; i < fs->first[idx].count; i++) {
        if (strcmp(fs->first[idx].symbols[i], "epsilon") == 0) {
            return 1;
        }
    }

    return 0;
}

SymbolSet get_first_set(FirstSets *fs, const char *symbol) {
    SymbolSet empty;
    empty.count = 0;
    
    // This function works with nonterminal indices stored in FirstSets
    // For now, return empty - should be accessed via the nonterminal index
    return empty;
}

SymbolSet get_follow_set(FollowSets *fos, const char *symbol) {
    SymbolSet empty;
    empty.count = 0;
    return empty;
}

void print_first_sets(FirstSets *fs, Grammar *g) {
    printf("\nFIRST SETS: \n");
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        printf("FIRST(%s) = { ", g->symbols.nonterminals[i]);
        for (int j = 0; j < fs->first[i].count; j++) {
            if (j > 0) printf(", ");
            printf("%s", fs->first[i].symbols[j]);
        }
        printf(" }\n");
    }
}

void print_follow_sets(FollowSets *fos, Grammar *g) {
    printf("\nFOLLOW SETS: \n");
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        printf("FOLLOW(%s) = { ", g->symbols.nonterminals[i]);
        for (int j = 0; j < fos->follow[i].count; j++) {
            if (j > 0) printf(", ");
            printf("%s", fos->follow[i].symbols[j]);
        }
        printf(" }\n");
    }
}

void free_first_sets(FirstSets *fs) {
    if (fs) free(fs);
}

void free_follow_sets(FollowSets *fos) {
    if (fos) free(fos);
}
