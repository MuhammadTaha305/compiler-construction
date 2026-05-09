#ifndef LL1_TABLE_H
#define LL1_TABLE_H

#include "grammar.h"
#include "first_follow.h"

#define SYNCH_ERROR -999

typedef struct {
    int production_idx;  // Index of production to use, or SYNCH_ERROR for sync error
} ParseTableEntry;

typedef struct {
    // Table[nonterminal][terminal] = production index
    ParseTableEntry table[MAX_SYMBOLS][MAX_SYMBOLS];
    int num_nonterminals;
    int num_terminals;
    Grammar *grammar;
} LL1ParseTable;

// LL(1) table construction
LL1ParseTable* build_ll1_table(Grammar *g, FirstSets *fs, FollowSets *fos);

// Check for conflicts
int check_ll1_conflicts(Grammar *g, FirstSets *fs, FollowSets *fos);

// Utility functions
void print_ll1_table(LL1ParseTable *table, Grammar *g);
void print_ll1_table_csv(LL1ParseTable *table, Grammar *g, const char *filename);
void free_ll1_table(LL1ParseTable *table);

#endif
