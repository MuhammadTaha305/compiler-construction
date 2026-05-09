#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 500
#define MAX_SCOPES 50
#define MAX_SYMBOL_LEN 100
#define MAX_TYPE_LEN 50

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_UNKNOWN
} DataType;

typedef struct {
    char name[MAX_SYMBOL_LEN];
    DataType type;
    char base_type[MAX_TYPE_LEN];
    int array_size;      // 0 if not an array
    int is_array;
    int is_declared;
    int scope_level;     // 0 = global, 1+ = local
    int line_num;
    char declaration_scope[MAX_SYMBOL_LEN]; // Function scope name
} SymbolInfo;

typedef struct {
    SymbolInfo symbols[MAX_SYMBOLS];
    int count;
} SymbolTable;

typedef struct {
    SymbolTable *table;
    int current_scope;
    int scope_stack[MAX_SCOPES];
    int scope_count;
    char scope_names[MAX_SCOPES][MAX_SYMBOL_LEN];
} ScopeManager;

typedef struct {
    int error_count;
    int warning_count;
    char error_messages[1000];  // For storing error messages
} SemanticError;

// Symbol table functions
SymbolTable* create_symbol_table();
void add_symbol(SymbolTable *table, const char *name, DataType type, int scope_level, const char *scope_name);
SymbolInfo* lookup_symbol(SymbolTable *table, const char *name, int current_scope);
SymbolInfo* lookup_symbol_global(SymbolTable *table, const char *name);
void print_symbol_table(SymbolTable *table);
void free_symbol_table(SymbolTable *table);

// Scope functions
ScopeManager* create_scope_manager(SymbolTable *table);
void push_scope(ScopeManager *sm, const char *scope_name);
void pop_scope(ScopeManager *sm);
int get_current_scope(ScopeManager *sm);
void print_scopes(ScopeManager *sm);
void free_scope_manager(ScopeManager *sm);

// Type checking functions
int is_compatible_type(DataType type1, DataType type2);
DataType promote_type(DataType type1, DataType type2);
const char* type_to_string(DataType type);
DataType string_to_type(const char *type_str);

// Semantic analysis functions
void check_type_compatibility(SymbolTable *table, DataType left, DataType right, int line_num, SemanticError *err);
void check_variable_declared(SymbolTable *table, const char *var_name, ScopeManager *sm, int line_num, SemanticError *err);
void check_type_mismatch(SymbolTable *table, const char *var_name, DataType expected, DataType actual, int line_num, SemanticError *err);

// Error handling
SemanticError* create_error_handler();
void add_error(SemanticError *err, const char *message);
void add_warning(SemanticError *err, const char *message);
void print_errors(SemanticError *err);
void free_error_handler(SemanticError *err);

#endif
