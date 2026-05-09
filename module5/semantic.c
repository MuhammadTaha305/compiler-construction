#include "semantic.h"

SymbolTable* create_symbol_table() {
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    table->count = 0;
    return table;
}

void add_symbol(SymbolTable *table, const char *name, DataType type, int scope_level, const char *scope_name) {
    if (table->count >= MAX_SYMBOLS) {
        fprintf(stderr, "Symbol table overflow\n");
        return;
    }

    // Check if symbol already exists at this scope level
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0 && 
            table->symbols[i].scope_level == scope_level &&
            strcmp(table->symbols[i].declaration_scope, scope_name) == 0) {
            fprintf(stderr, "ERROR: Redeclaration of '%s' at scope %d\n", name, scope_level);
            return;
        }
    }

    SymbolInfo *sym = &table->symbols[table->count++];
    strcpy(sym->name, name);
    sym->type = type;
    strcpy(sym->base_type, type_to_string(type));
    sym->array_size = 0;
    sym->is_array = 0;
    sym->is_declared = 1;
    sym->scope_level = scope_level;
    sym->line_num = 0;
    strcpy(sym->declaration_scope, scope_name);
}

SymbolInfo* lookup_symbol(SymbolTable *table, const char *name, int current_scope) {
    // Look for symbol starting from current scope and moving outward
    for (int i = table->count - 1; i >= 0; i--) {
        if (strcmp(table->symbols[i].name, name) == 0 && 
            table->symbols[i].scope_level <= current_scope) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

SymbolInfo* lookup_symbol_global(SymbolTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

void print_symbol_table(SymbolTable *table) {
    printf("\n---SYMBOL TABLE---\n");
    printf("%-20s %-15s %-8s %-15s %-10s\n", "Name", "Type", "Scope", "Declaration Scope", "Declared");
    printf("-------------------------------------------------------------------------------------\n");

    for (int i = 0; i < table->count; i++) {
        SymbolInfo *sym = &table->symbols[i];
        printf("%-20s %-15s %-8d %-15s %-10s\n",
               sym->name,
               sym->base_type,
               sym->scope_level,
               sym->declaration_scope,
               sym->is_declared ? "Yes" : "No");
    }
}

void free_symbol_table(SymbolTable *table) {
    if (table) free(table);
}

ScopeManager* create_scope_manager(SymbolTable *table) {
    ScopeManager *sm = (ScopeManager *)malloc(sizeof(ScopeManager));
    sm->table = table;
    sm->current_scope = 0;
    sm->scope_count = 0;
    sm->scope_stack[sm->scope_count] = 0;
    strcpy(sm->scope_names[sm->scope_count], "GLOBAL");
    sm->scope_count++;
    return sm;
}

void push_scope(ScopeManager *sm, const char *scope_name) {
    if (sm->scope_count >= MAX_SCOPES) {
        fprintf(stderr, "Scope stack overflow\n");
        return;
    }

    sm->current_scope++;
    sm->scope_stack[sm->scope_count] = sm->current_scope;
    strcpy(sm->scope_names[sm->scope_count], scope_name);
    sm->scope_count++;

    printf("→ Entering scope: %s (level %d)\n", scope_name, sm->current_scope);
}

void pop_scope(ScopeManager *sm) {
    if (sm->scope_count <= 1) {
        fprintf(stderr, "Cannot pop global scope\n");
        return;
    }

    printf("← Exiting scope: %s (level %d)\n", 
           sm->scope_names[sm->scope_count-1], 
           sm->current_scope);

    sm->scope_count--;
    sm->current_scope = sm->scope_stack[sm->scope_count-1];
}

int get_current_scope(ScopeManager *sm) {
    return sm->current_scope;
}

void print_scopes(ScopeManager *sm) {
    printf("\nScope Stack:\n");
    for (int i = 0; i < sm->scope_count; i++) {
        printf("  Level %d: %s\n", sm->scope_stack[i], sm->scope_names[i]);
    }
}

void free_scope_manager(ScopeManager *sm) {
    if (sm) free(sm);
}

int is_compatible_type(DataType type1, DataType type2) {
    if (type1 == type2) return 1;
    if (type1 == TYPE_UNKNOWN || type2 == TYPE_UNKNOWN) return 1;
    
    // Allow implicit int to float conversion
    if ((type1 == TYPE_INT && type2 == TYPE_FLOAT) ||
        (type1 == TYPE_FLOAT && type2 == TYPE_INT)) {
        return 1;
    }
    
    return 0;
}

DataType promote_type(DataType type1, DataType type2) {
    if (type1 == TYPE_FLOAT || type2 == TYPE_FLOAT) return TYPE_FLOAT;
    if (type1 == TYPE_INT || type2 == TYPE_INT) return TYPE_INT;
    return TYPE_UNKNOWN;
}

const char* type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT:      return "int";
        case TYPE_FLOAT:    return "float";
        case TYPE_CHAR:     return "char";
        case TYPE_BOOL:     return "bool";
        case TYPE_VOID:     return "void";
        case TYPE_ARRAY:    return "array";
        case TYPE_FUNCTION: return "function";
        case TYPE_UNKNOWN:  return "unknown";
        default:            return "unknown";
    }
}

DataType string_to_type(const char *type_str) {
    if (strcmp(type_str, "int") == 0)      return TYPE_INT;
    if (strcmp(type_str, "float") == 0)    return TYPE_FLOAT;
    if (strcmp(type_str, "char") == 0)     return TYPE_CHAR;
    if (strcmp(type_str, "bool") == 0)     return TYPE_BOOL;
    if (strcmp(type_str, "void") == 0)     return TYPE_VOID;
    if (strcmp(type_str, "array") == 0)    return TYPE_ARRAY;
    if (strcmp(type_str, "function") == 0) return TYPE_FUNCTION;
    return TYPE_UNKNOWN;
}

void check_type_compatibility(SymbolTable *table, DataType left, DataType right, int line_num, SemanticError *err) {
    if (!is_compatible_type(left, right)) {
        char msg[300];
        snprintf(msg, sizeof(msg), "Line %d: Type mismatch - %s is not compatible with %s",
                 line_num, type_to_string(left), type_to_string(right));
        add_error(err, msg);
    }
}

void check_variable_declared(SymbolTable *table, const char *var_name, ScopeManager *sm, int line_num, SemanticError *err) {
    SymbolInfo *sym = lookup_symbol(table, var_name, get_current_scope(sm));
    if (!sym || !sym->is_declared) {
        char msg[300];
        snprintf(msg, sizeof(msg), "Line %d: Variable '%s' used before declaration", line_num, var_name);
        add_error(err, msg);
    }
}

void check_type_mismatch(SymbolTable *table, const char *var_name, DataType expected, DataType actual, int line_num, SemanticError *err) {
    if (!is_compatible_type(expected, actual)) {
        char msg[300];
        snprintf(msg, sizeof(msg), "Line %d: Type mismatch for '%s' - expected %s but got %s",
                 line_num, var_name, type_to_string(expected), type_to_string(actual));
        add_error(err, msg);
    }
}

SemanticError* create_error_handler() {
    SemanticError *err = (SemanticError *)malloc(sizeof(SemanticError));
    err->error_count = 0;
    err->warning_count = 0;
    err->error_messages[0] = '\0';
    return err;
}

void add_error(SemanticError *err, const char *message) {
    err->error_count++;
    strcat(err->error_messages, "ERROR: ");
    strcat(err->error_messages, message);
    strcat(err->error_messages, "\n");
}

void add_warning(SemanticError *err, const char *message) {
    err->warning_count++;
    strcat(err->error_messages, "WARNING: ");
    strcat(err->error_messages, message);
    strcat(err->error_messages, "\n");
}

void print_errors(SemanticError *err) {
    printf("\n---SEMANTIC ANALYSIS REPORT---\n");
    printf("Errors: %d\n", err->error_count);
    printf("Warnings: %d\n\n", err->warning_count);

    if (strlen(err->error_messages) > 0) {
        printf("%s", err->error_messages);
    } else {
        printf("No errors or warnings\n");
    }
}

void free_error_handler(SemanticError *err) {
    if (err) free(err);
}
