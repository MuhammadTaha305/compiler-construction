# MODULE 5: SEMANTIC ANALYSIS (TYPE & SCOPE CHECKING)

## Overview
This module implements semantic analysis for compiler construction, including symbol table management, type checking, and scope checking. It verifies that the program is logically valid after parsing.

## Components

### 1. Symbol Table
Maintains information about declared variables with:
- Variable name
- Data type (int, float, char, bool, void, array, function)
- Scope level (global = 0, local > 0)
- Declaration scope (function name)
- Declaration status

### 2. Scope Manager
Manages nested scopes with:
- Scope stack for tracking nesting depth
- Scope names for context identification
- Automatic scope entry/exit
- Local scope precedence over global scope

### 3. Type Checker
Performs type compatibility checking:
- Verifies operations use compatible types
- Allows implicit int↔float conversions
- Type promotion rules
- Detects type mismatches

### 4. Scope Checker
Verifies variable usage:
- Checks variables are declared before use
- Enforces scope visibility rules
- Local scope hides global scope variables
- Detects undefined variable usage

## Data Types Supported
- `int` - Integer type
- `float` - Floating-point type
- `char` - Character type
- `bool` - Boolean type
- `void` - No return type
- `array` - Array type
- `function` - Function type

## Type Compatibility Rules
- Same types are compatible
- int and float are compatible (implicit conversion)
- Explicit casting available
- Type unknown is compatible with everything

## Semantic Checks Performed

### 1. Type Checking
```c
// Check operation compatibility
check_type_compatibility(table, TYPE_INT, TYPE_FLOAT, line_num, err);

// Check variable assignment type
check_type_mismatch(table, "x", TYPE_INT, TYPE_FLOAT, line_num, err);
```

### 2. Scope Checking
```c
// Check variable declared
check_variable_declared(table, "var_name", scope_manager, line_num, err);

// Local scope shadowing
push_scope(sm, "function_name");
// ... use variables ...
pop_scope(sm);
```

## Program Files

### 1. semantic.h/c
Core semantic analysis implementation:
- Symbol table functions
- Scope management functions
- Type checking functions
- Error reporting

### 2. semantic_test.c
Comprehensive test demonstrating:
- Variable declaration in global scope
- Type compatibility checking
- Undeclared variable detection
- Function scope entry/exit
- Local variable shadowing
- Nested scope visibility

## Usage Example

```c
// Create symbol table and scope manager
SymbolTable *table = create_symbol_table();
ScopeManager *sm = create_scope_manager(table);
SemanticError *err = create_error_handler();

// Declare global variables
add_symbol(table, "x", TYPE_INT, 0, "GLOBAL");
add_symbol(table, "y", TYPE_FLOAT, 0, "GLOBAL");

// Enter function scope
push_scope(sm, "main");
add_symbol(table, "local_var", TYPE_INT, 1, "main");

// Check variable declared
check_variable_declared(table, "x", sm, 1, err);
check_variable_declared(table, "undefined", sm, 2, err);

// Check type compatibility
check_type_compatibility(table, TYPE_INT, TYPE_FLOAT, 3, err);

// Exit function scope
pop_scope(sm);

// Print results
print_symbol_table(table);
print_errors(err);
```

## Error Handling

Three levels of diagnostics:
1. **Errors** - Semantic violations (e.g., undeclared variable)
2. **Warnings** - Suspicious code (e.g., unused variable)
3. **No errors** - Clean semantic analysis

## Test Cases

### Test Case 1: Type Checking and Scope Checking
- Global scope: declare `int x`, `float y`
- Check if `x` is declared: ✓
- Check if undefined `z` is declared: ✗ (error)
- Check int vs float compatibility: ✓

### Test Case 2: Function Scope
- Enter `main` function
- Declare `int local_var`
- Check local variable accessible: ✓

### Test Case 3: Nested Scope
- Enter `if_block` within `main`
- Declare `char nested_var`
- Check nested variable accessible: ✓
- Check outer scope variable accessible: ✓ (outer visible in nested)

## Symbol Table Output Example
```
===== SYMBOL TABLE =====
Name                 Type            Scope    Declaration Scope   Declared
x                    int             0        GLOBAL              Yes
y                    float           0        GLOBAL              Yes
local_var            int             1        main                Yes
nested_var           char            2        if_block            Yes
```

## Deliverables
1. **Source files**: semantic.h/c implementing symbol table, type checker, scope checker
2. **Test program**: semantic_test.c demonstrating correct error detection
3. **Documentation**: This README with usage examples

## Notes
- Symbol table uses linear search (adequate for educational purposes)
- Scope stack limited to 50 nesting levels
- Symbol count limited to 500 entries
- Type promotion follows standard C rules
