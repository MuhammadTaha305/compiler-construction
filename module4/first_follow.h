#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include "grammar.h"

typedef struct {
    SymbolSet first[MAX_SYMBOLS];
    int count;
} FirstSets;

typedef struct {
    SymbolSet follow[MAX_SYMBOLS];
    int count;
} FollowSets;

// FIRST set computation
FirstSets* compute_first_sets(Grammar *g);
SymbolSet get_first_set(FirstSets *fs, const char *symbol);

// FOLLOW set computation
FollowSets* compute_follow_sets(Grammar *g, FirstSets *fs);
SymbolSet get_follow_set(FollowSets *fos, const char *symbol);

// Nullable check
int is_nullable(Grammar *g, FirstSets *fs, const char *symbol);

// Utility functions
void print_first_sets(FirstSets *fs, Grammar *g);
void print_follow_sets(FollowSets *fos, Grammar *g);
void free_first_sets(FirstSets *fs);
void free_follow_sets(FollowSets *fos);

#endif
