#include "grammar.h"
#include <ctype.h>

Grammar* parse_grammar_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }

    Grammar *g = (Grammar *)malloc(sizeof(Grammar));
    g->num_productions = 0;
    g->symbols.num_terminals = 0;
    g->symbols.num_nonterminals = 0;

    char line[500];
    int is_start_set = 0;

    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        // Remove trailing whitespace
        int len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' || line[len-1] == ' ' || line[len-1] == '\t')) {
            line[--len] = '\0';
        }

        if (len == 0) continue;

        // Parse production: "LHS -> RHS1 RHS2 ... | RHS1 RHS2 ... | epsilon"
        char *arrow = strstr(line, "->");
        if (!arrow) continue;

        // Extract LHS
        char lhs[MAX_SYMBOL_LEN];
        strncpy(lhs, line, arrow - line);
        lhs[arrow - line] = '\0';
        
        // Trim whitespace from lhs
        int i = strlen(lhs) - 1;
        while (i >= 0 && (lhs[i] == ' ' || lhs[i] == '\t')) lhs[i--] = '\0';
        i = 0;
        while (i < strlen(lhs) && (lhs[i] == ' ' || lhs[i] == '\t')) i++;
        if (i > 0) memmove(lhs, lhs + i, strlen(lhs) - i + 1);

        if (!is_start_set) {
            strcpy(g->start_symbol, lhs);
            is_start_set = 1;
        }

        add_symbol(&g->symbols, lhs, 0); // lhs is nonterminal

        // Process alternatives (split by |)
        char *rhs_start = arrow + 2;
        char *pipe = rhs_start;

        while (pipe) {
            // Find next pipe
            pipe = strchr(rhs_start, '|');
            int prod_len = pipe ? pipe - rhs_start : strlen(rhs_start);

            // Extract production
            char prod[300];
            strncpy(prod, rhs_start, prod_len);
            prod[prod_len] = '\0';

            // Trim whitespace
            i = 0;
            while (i < strlen(prod) && (prod[i] == ' ' || prod[i] == '\t')) i++;
            if (i > 0) memmove(prod, prod + i, strlen(prod) - i + 1);
            
            i = strlen(prod) - 1;
            while (i >= 0 && (prod[i] == ' ' || prod[i] == '\t')) prod[i--] = '\0';

            if (strlen(prod) > 0) {
                Production *p = &g->productions[g->num_productions];
                strcpy(p->lhs, lhs);

                // Check for epsilon
                if (strcmp(prod, "epsilon") == 0 || strcmp(prod, "ε") == 0) {
                    p->is_epsilon = 1;
                    p->rhs_count = 0;
                } else {
                    p->is_epsilon = 0;
                    p->rhs_count = 0;

                    // Tokenize RHS by spaces
                    char prod_copy[300];
                    strcpy(prod_copy, prod);
                    char *token = strtok(prod_copy, " \t");
                    
                    while (token && p->rhs_count < MAX_RHS) {
                        strcpy(p->rhs[p->rhs_count++], token);
                        
                        // Determine if terminal or nonterminal
                        if (token[0] == '\'' || token[0] == '"') {
                            // Quoted terminal
                            add_symbol(&g->symbols, token, 1);
                        } else if (strlen(token) == 1 && !isalpha(token[0])) {
                            // Single character terminal
                            add_symbol(&g->symbols, token, 1);
                        } else if (isupper(token[0]) || (token[0] >= 'A' && token[0] <= 'Z')) {
                            // Uppercase = nonterminal (convention)
                            add_symbol(&g->symbols, token, 0);
                        } else {
                            // Lowercase/mixed = terminal
                            add_symbol(&g->symbols, token, 1);
                        }
                        
                        token = strtok(NULL, " \t");
                    }
                }

                g->num_productions++;
                if (g->num_productions >= MAX_PRODUCTIONS) break;
            }

            if (pipe) rhs_start = pipe + 1;
            else break;
        }
    }

    fclose(file);
    return g;
}

void add_production(Grammar *g, const char *lhs, const char **rhs, int rhs_len) {
    if (g->num_productions >= MAX_PRODUCTIONS) return;

    Production *p = &g->productions[g->num_productions++];
    strcpy(p->lhs, lhs);
    p->rhs_count = rhs_len;
    p->is_epsilon = (rhs_len == 0);

    for (int i = 0; i < rhs_len; i++) {
        strcpy(p->rhs[i], rhs[i]);
    }
}

void add_symbol(SymbolTable *st, const char *symbol, int is_terminal) {
    // Check if already exists
    if (is_terminal) {
        for (int i = 0; i < st->num_terminals; i++) {
            if (strcmp(st->terminals[i], symbol) == 0) return;
        }
        if (st->num_terminals < MAX_SYMBOLS) {
            strcpy(st->terminals[st->num_terminals++], symbol);
        }
    } else {
        for (int i = 0; i < st->num_nonterminals; i++) {
            if (strcmp(st->nonterminals[i], symbol) == 0) return;
        }
        if (st->num_nonterminals < MAX_SYMBOLS) {
            strcpy(st->nonterminals[st->num_nonterminals++], symbol);
        }
    }
}

int is_terminal(Grammar *g, const char *symbol) {
    for (int i = 0; i < g->symbols.num_terminals; i++) {
        if (strcmp(g->symbols.terminals[i], symbol) == 0) return 1;
    }
    return 0;
}

int is_nonterminal(Grammar *g, const char *symbol) {
    for (int i = 0; i < g->symbols.num_nonterminals; i++) {
        if (strcmp(g->symbols.nonterminals[i], symbol) == 0) return 1;
    }
    return 0;
}

void print_grammar(Grammar *g) {
    printf("\n===== GRAMMAR =====\n");
    printf("Start symbol: %s\n\n", g->start_symbol);
    printf("Productions:\n");
    for (int i = 0; i < g->num_productions; i++) {
        Production *p = &g->productions[i];
        printf("%d. %s -> ", i+1, p->lhs);
        if (p->is_epsilon) {
            printf("ε");
        } else {
            for (int j = 0; j < p->rhs_count; j++) {
                if (j > 0) printf(" ");
                printf("%s", p->rhs[j]);
            }
        }
        printf("\n");
    }
}

void free_grammar(Grammar *g) {
    if (g) free(g);
}

void set_add(SymbolSet *set, const char *symbol) {
    if (!set_contains(set, symbol) && set->count < MAX_SYMBOLS) {
        strcpy(set->symbols[set->count++], symbol);
    }
}

int set_contains(SymbolSet *set, const char *symbol) {
    for (int i = 0; i < set->count; i++) {
        if (strcmp(set->symbols[i], symbol) == 0) return 1;
    }
    return 0;
}

void set_copy(SymbolSet *dest, SymbolSet *src) {
    dest->count = 0;
    for (int i = 0; i < src->count; i++) {
        set_add(dest, src->symbols[i]);
    }
}

void set_union(SymbolSet *dest, SymbolSet *src) {
    for (int i = 0; i < src->count; i++) {
        set_add(dest, src->symbols[i]);
    }
}

void set_print(SymbolSet *set, const char *name) {
    printf("%s: { ", name);
    for (int i = 0; i < set->count; i++) {
        if (i > 0) printf(", ");
        printf("%s", set->symbols[i]);
    }
    printf(" }\n");
}
