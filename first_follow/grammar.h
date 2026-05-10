#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 100
#define MAX_PRODUCTIONS 200
#define MAX_RHS 50
#define MAX_SYMBOL_LEN 50

typedef struct {
    char terminals[MAX_SYMBOLS][MAX_SYMBOL_LEN];
    int num_terminals;
    char nonterminals[MAX_SYMBOLS][MAX_SYMBOL_LEN];
    int num_nonterminals;
} SymbolTable;

typedef struct {
    char lhs[MAX_SYMBOL_LEN];
    char rhs[MAX_RHS][MAX_SYMBOL_LEN];
    int rhs_count;
    int is_epsilon;
} Production;

typedef struct {
    Production productions[MAX_PRODUCTIONS];
    int num_productions;
    char start_symbol[MAX_SYMBOL_LEN];
    SymbolTable symbols;
} Grammar;

typedef struct {
    char symbols[MAX_SYMBOLS][MAX_SYMBOL_LEN];
    int count;
} SymbolSet;

// Function declarations
Grammar* parse_grammar_from_file(const char *filename);
void add_production(Grammar *g, const char *lhs, const char **rhs, int rhs_len);
void add_symbol(SymbolTable *st, const char *symbol, int is_terminal);
int is_terminal(Grammar *g, const char *symbol);
int is_nonterminal(Grammar *g, const char *symbol);
void print_grammar(Grammar *g);
void free_grammar(Grammar *g);

// Set operations
void set_add(SymbolSet *set, const char *symbol);
int set_contains(SymbolSet *set, const char *symbol);
void set_copy(SymbolSet *dest, SymbolSet *src);
void set_union(SymbolSet *dest, SymbolSet *src);
void set_print(SymbolSet *set, const char *name);

#endif
