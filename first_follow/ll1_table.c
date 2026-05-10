#include "ll1_table.h"
#include <ctype.h>

LL1ParseTable* build_ll1_table(Grammar *g, FirstSets *fs, FollowSets *fos) {
    LL1ParseTable *table = (LL1ParseTable *)malloc(sizeof(LL1ParseTable));
    table->grammar = g;
    table->num_nonterminals = g->symbols.num_nonterminals;
    table->num_terminals = g->symbols.num_terminals;

    // Initialize table with empty entries
    for (int i = 0; i < table->num_nonterminals; i++) {
        for (int j = 0; j < table->num_terminals; j++) {
            table->table[i][j].production_idx = -1;
        }
    }

    // For each production A -> α
    for (int p = 0; p < g->num_productions; p++) {
        Production *prod = &g->productions[p];

        // Find nonterminal index for A
        int a_idx = -1;
        for (int i = 0; i < g->symbols.num_nonterminals; i++) {
            if (strcmp(g->symbols.nonterminals[i], prod->lhs) == 0) {
                a_idx = i;
                break;
            }
        }

        if (a_idx == -1) continue;

        // Compute FIRST(α)
        SymbolSet first_alpha;
        first_alpha.count = 0;

        if (prod->is_epsilon) {
            // FIRST(epsilon) is empty
        } else {
            int all_nullable = 1;
            for (int i = 0; i < prod->rhs_count; i++) {
                char *X = prod->rhs[i];

                if (is_terminal(g, X)) {
                    set_add(&first_alpha, X);
                    all_nullable = 0;
                } else if (is_nonterminal(g, X)) {
                    int x_idx = -1;
                    for (int j = 0; j < g->symbols.num_nonterminals; j++) {
                        if (strcmp(g->symbols.nonterminals[j], X) == 0) {
                            x_idx = j;
                            break;
                        }
                    }

                    if (x_idx != -1) {
                        int x_nullable = 0;
                        for (int j = 0; j < fs->first[x_idx].count; j++) {
                            if (strcmp(fs->first[x_idx].symbols[j], "epsilon") == 0) {
                                x_nullable = 1;
                            } else {
                                set_add(&first_alpha, fs->first[x_idx].symbols[j]);
                            }
                        }
                        if (!x_nullable) {
                            all_nullable = 0;
                        }
                    }
                }

                if (!all_nullable) break;
            }
        }

        // For each terminal 'a' in FIRST(α), add production to M[A, a]
        for (int i = 0; i < first_alpha.count; i++) {
            char *terminal = first_alpha.symbols[i];
            
            // Find terminal index
            int t_idx = -1;
            for (int j = 0; j < g->symbols.num_terminals; j++) {
                if (strcmp(g->symbols.terminals[j], terminal) == 0) {
                    t_idx = j;
                    break;
                }
            }

            if (t_idx != -1) {
                if (table->table[a_idx][t_idx].production_idx == -1) {
                    table->table[a_idx][t_idx].production_idx = p;
                } else {
                    // Conflict detected
                    printf("WARNING: Conflict at M[%s, %s]\n", prod->lhs, terminal);
                }
            }
        }

        // If α is nullable, for each terminal 'b' in FOLLOW(A), add production to M[A, b]
        int alpha_nullable = 0;
        if (prod->is_epsilon) {
            alpha_nullable = 1;
        } else {
            alpha_nullable = 1;
            for (int i = 0; i < prod->rhs_count; i++) {
                char *X = prod->rhs[i];
                int x_nullable = 0;

                if (is_nonterminal(g, X)) {
                    int x_idx = -1;
                    for (int j = 0; j < g->symbols.num_nonterminals; j++) {
                        if (strcmp(g->symbols.nonterminals[j], X) == 0) {
                            x_idx = j;
                            break;
                        }
                    }
                    if (x_idx != -1) {
                        for (int j = 0; j < fs->first[x_idx].count; j++) {
                            if (strcmp(fs->first[x_idx].symbols[j], "epsilon") == 0) {
                                x_nullable = 1;
                                break;
                            }
                        }
                    }
                }

                if (!x_nullable) {
                    alpha_nullable = 0;
                    break;
                }
            }
        }

        if (alpha_nullable) {
            for (int j = 0; j < fos->follow[a_idx].count; j++) {
                char *b = fos->follow[a_idx].symbols[j];
                
                // Find terminal index
                int t_idx = -1;
                for (int k = 0; k < g->symbols.num_terminals; k++) {
                    if (strcmp(g->symbols.terminals[k], b) == 0) {
                        t_idx = k;
                        break;
                    }
                }

                if (t_idx != -1) {
                    if (table->table[a_idx][t_idx].production_idx == -1) {
                        table->table[a_idx][t_idx].production_idx = p;
                    } else if (table->table[a_idx][t_idx].production_idx != p) {
                        // Conflict detected
                        printf("WARNING: Conflict at M[%s, %s]\n", prod->lhs, b);
                    }
                }
            }
        }
    }

    return table;
}

int check_ll1_conflicts(Grammar *g, FirstSets *fs, FollowSets *fos) {
    int conflicts = 0;

    // For each nonterminal
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        char *A = g->symbols.nonterminals[i];
        
        // Get all productions for A
        SymbolSet processed_pairs;
        processed_pairs.count = 0;

        for (int p1 = 0; p1 < g->num_productions; p1++) {
            if (strcmp(g->productions[p1].lhs, A) != 0) continue;

            Production *prod1 = &g->productions[p1];

            for (int p2 = p1 + 1; p2 < g->num_productions; p2++) {
                if (strcmp(g->productions[p2].lhs, A) != 0) continue;

                Production *prod2 = &g->productions[p2];

                // Compute FIRST(prod1) and FIRST(prod2)
                SymbolSet first1, first2;
                first1.count = 0;
                first2.count = 0;

                // FIRST(prod1)
                if (!prod1->is_epsilon) {
                    int all_nullable = 1;
                    for (int j = 0; j < prod1->rhs_count; j++) {
                        char *X = prod1->rhs[j];
                        if (is_terminal(g, X)) {
                            set_add(&first1, X);
                            all_nullable = 0;
                        } else if (is_nonterminal(g, X)) {
                            int x_idx = -1;
                            for (int k = 0; k < g->symbols.num_nonterminals; k++) {
                                if (strcmp(g->symbols.nonterminals[k], X) == 0) {
                                    x_idx = k;
                                    break;
                                }
                            }
                            if (x_idx != -1) {
                                int x_nullable = 0;
                                for (int k = 0; k < fs->first[x_idx].count; k++) {
                                    if (strcmp(fs->first[x_idx].symbols[k], "epsilon") != 0) {
                                        set_add(&first1, fs->first[x_idx].symbols[k]);
                                    } else {
                                        x_nullable = 1;
                                    }
                                }
                                if (!x_nullable) all_nullable = 0;
                            }
                        }
                        if (!all_nullable) break;
                    }
                }

                // FIRST(prod2)
                if (!prod2->is_epsilon) {
                    int all_nullable = 1;
                    for (int j = 0; j < prod2->rhs_count; j++) {
                        char *X = prod2->rhs[j];
                        if (is_terminal(g, X)) {
                            set_add(&first2, X);
                            all_nullable = 0;
                        } else if (is_nonterminal(g, X)) {
                            int x_idx = -1;
                            for (int k = 0; k < g->symbols.num_nonterminals; k++) {
                                if (strcmp(g->symbols.nonterminals[k], X) == 0) {
                                    x_idx = k;
                                    break;
                                }
                            }
                            if (x_idx != -1) {
                                int x_nullable = 0;
                                for (int k = 0; k < fs->first[x_idx].count; k++) {
                                    if (strcmp(fs->first[x_idx].symbols[k], "epsilon") != 0) {
                                        set_add(&first2, fs->first[x_idx].symbols[k]);
                                    } else {
                                        x_nullable = 1;
                                    }
                                }
                                if (!x_nullable) all_nullable = 0;
                            }
                        }
                        if (!all_nullable) break;
                    }
                }

                // Check for conflict
                for (int j = 0; j < first1.count; j++) {
                    if (set_contains(&first2, first1.symbols[j])) {
                        conflicts++;
                        printf("Conflict: %s -> ... and %s -> ... both contain %s in FIRST\n",
                               A, A, first1.symbols[j]);
                    }
                }
            }
        }
    }

    return conflicts;
}

void print_ll1_table(LL1ParseTable *table, Grammar *g) {
    printf("\nLL(1) PARSING TABLE: \n\n");
    
    int col_width = 8;
    
    // Print header row
    printf("%-12s", "A\\a");
    printf("|");
    for (int j = 0; j < g->symbols.num_terminals; j++) {
        printf(" %*s |", col_width, g->symbols.terminals[j]);
    }
    printf("\n");

    // Print separator line
    printf("-----------");
    for (int j = 0; j < g->symbols.num_terminals; j++) {
        printf("----------");
    }
    printf("\n");

    // Print table rows
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        printf("%-12s", g->symbols.nonterminals[i]);
        printf("|");
        for (int j = 0; j < g->symbols.num_terminals; j++) {
            int prod_idx = table->table[i][j].production_idx;
            if (prod_idx == -1) {
                printf(" %*s |", col_width, "");
            } else {
                char cell[20];
                snprintf(cell, sizeof(cell), "[%d]", prod_idx);
                printf(" %*s |", col_width, cell);
            }
        }
        printf("\n");
    }

    // Print legend with productions
    printf("\nPRODUCTIONS: \n");
    for (int i = 0; i < g->num_productions; i++) {
        Production *p = &g->productions[i];
        printf("[%d] %s -> ", i, p->lhs);
        if (p->is_epsilon) {
            printf("epsilon");
        } else {
            for (int j = 0; j < p->rhs_count; j++) {
                if (j > 0) printf(" ");
                printf("%s", p->rhs[j]);
            }
        }
        printf("\n");
    }
}

void print_ll1_table_csv(LL1ParseTable *table, Grammar *g, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return;
    }

    // Header
    fprintf(f, "Nonterminal");
    for (int j = 0; j < g->symbols.num_terminals; j++) {
        fprintf(f, ",%s", g->symbols.terminals[j]);
    }
    fprintf(f, "\n");

    // Table
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        fprintf(f, "%s", g->symbols.nonterminals[i]);
        for (int j = 0; j < g->symbols.num_terminals; j++) {
            int prod_idx = table->table[i][j].production_idx;
            if (prod_idx == -1) {
                fprintf(f, ",");
            } else {
                fprintf(f, ",%d", prod_idx);
            }
        }
        fprintf(f, "\n");
    }

    fclose(f);
}

void free_ll1_table(LL1ParseTable *table) {
    if (table) free(table);
}
