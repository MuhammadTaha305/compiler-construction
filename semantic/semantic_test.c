#include "semantic.h"

int main() {
    // Create symbol table and scope manager
    SymbolTable *table = create_symbol_table();
    ScopeManager *sm = create_scope_manager(table);
    SemanticError *err = create_error_handler();

    printf("\n--- TEST CASE 1: Type Checking and Scope Checking ---\n\n");

    // Global scope: declare variables
    printf("Global scope:\n");
    add_symbol(table, "x", TYPE_INT, 0, "GLOBAL");
    printf("  Declared: int x\n");
    add_symbol(table, "y", TYPE_FLOAT, 0, "GLOBAL");
    printf("  Declared: float y\n");

    // Check variable declared
    printf("\n  Checking if 'x' is declared: ");
    check_variable_declared(table, "x", sm, 1, err);
    if (err->error_count == 0) printf("✓ OK\n");
    else printf("Error\n");

    // Check undeclared variable
    printf("  Checking if 'z' is declared: ");
    int err_count_before = err->error_count;
    check_variable_declared(table, "z", sm, 2, err);
    if (err->error_count > err_count_before) printf("Error detected (as expected)\n");
    else printf("OK\n");

    // Type compatibility
    printf("\n  Checking type compatibility (int vs float): ");
    err_count_before = err->error_count;
    check_type_compatibility(table, TYPE_INT, TYPE_FLOAT, 3, err);
    if (err->error_count > err_count_before) printf("✗ Error\n");
    else printf("Compatible\n");

    // Type mismatch
    printf("  Checking type compatibility (int vs void): ");
    err_count_before = err->error_count;
    check_type_compatibility(table, TYPE_INT, TYPE_VOID, 4, err);
    if (err->error_count > err_count_before) printf("✗ Error detected (as expected)\n");
    else printf("OK\n");

    // Enter function scope
    printf("\n--- TEST CASE 2: Function Scope ---\n\n");
    push_scope(sm, "main");
    add_symbol(table, "local_var", TYPE_INT, 1, "main");
    printf("  Declared in main: int local_var\n");

    // Check local scope shadowing
    printf("  Checking if 'local_var' is accessible in main: ");
    SymbolInfo *sym = lookup_symbol(table, "local_var", get_current_scope(sm));
    if (sym && sym->is_declared) printf("OK\n");
    else printf("Not found\n");

    // Nested scope
    printf("\n--- TEST CASE 3: Nested Scope ---\n\n");
    push_scope(sm, "if_block");
    add_symbol(table, "nested_var", TYPE_CHAR, 2, "if_block");
    printf("  Declared in if_block: char nested_var\n");

    printf("  Checking if 'nested_var' is accessible: ");
    sym = lookup_symbol(table, "nested_var", get_current_scope(sm));
    if (sym && sym->is_declared) printf("OK\n");
    else printf("Not found\n");

    printf("  Checking if 'local_var' is accessible in nested scope: ");
    sym = lookup_symbol(table, "local_var", get_current_scope(sm));
    if (sym && sym->is_declared) printf("OK (outer scope visible)\n");
    else printf("Not found\n");

    pop_scope(sm);
    pop_scope(sm);

    // Print results
    print_symbol_table(table);
    print_scopes(sm);
    print_errors(err);

    // Cleanup
    free_error_handler(err);
    free_scope_manager(sm);
    free_symbol_table(table);

    return 0;
}
