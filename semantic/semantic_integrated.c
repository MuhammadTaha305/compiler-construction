/*
 * MODULE 5: SEMANTIC ANALYSIS (Integrated Version)
 * Mini-Compiler Project - CS-346 Compiler Construction
 *
 * Type checking and scope analysis integrated with pipeline.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../common/common.h"

/* Semantic analyzer state */
typedef struct {
    SymbolTable *symbols;
    int current_scope;
    int scope_stack[MAX_SCOPES];
    char scope_names[MAX_SCOPES][MAX_TOKEN_LEN];
    int scope_count;
    int error_count;
    int warning_count;
    char errors[10000];
    char warnings[5000];
} SemanticAnalyzer;

/* Forward declarations */
static void analyze_node(SemanticAnalyzer *sa, ASTNode *node);
static DataType analyze_expression(SemanticAnalyzer *sa, ASTNode *node);
static void analyze_function(SemanticAnalyzer *sa, ASTNode *node);
static void analyze_block(SemanticAnalyzer *sa, ASTNode *node);
static void analyze_statement(SemanticAnalyzer *sa, ASTNode *node);

/* Add built-in function to symbol table */
static void add_builtin_function(SymbolTable *table, const char *name, DataType return_type) {
    Symbol *sym = &table->symbols[table->count++];
    strcpy(sym->name, name);
    sym->type = return_type;
    strcpy(sym->type_name, data_type_to_string(return_type));
    sym->scope_level = 0;
    sym->is_function = 1;
    sym->is_array = 0;
    sym->line_declared = 0;
    strcpy(sym->scope_name, "builtin");
}

/* Create semantic analyzer */
SemanticAnalyzer* create_semantic_analyzer(void) {
    SemanticAnalyzer *sa = (SemanticAnalyzer *)malloc(sizeof(SemanticAnalyzer));
    sa->symbols = create_symbol_table();
    sa->current_scope = 0;
    sa->scope_count = 1;
    sa->scope_stack[0] = 0;
    strcpy(sa->scope_names[0], "global");
    sa->error_count = 0;
    sa->warning_count = 0;
    sa->errors[0] = '\0';
    sa->warnings[0] = '\0';

    /* Add built-in functions */
    add_builtin_function(sa->symbols, "print", TYPE_VOID);
    add_builtin_function(sa->symbols, "printf", TYPE_INT);
    add_builtin_function(sa->symbols, "scanf", TYPE_INT);
    add_builtin_function(sa->symbols, "log", TYPE_FLOAT);
    add_builtin_function(sa->symbols, "exp", TYPE_FLOAT);
    add_builtin_function(sa->symbols, "sqrt", TYPE_FLOAT);
    add_builtin_function(sa->symbols, "sin", TYPE_FLOAT);
    add_builtin_function(sa->symbols, "cos", TYPE_FLOAT);
    add_builtin_function(sa->symbols, "tan", TYPE_FLOAT);
    add_builtin_function(sa->symbols, "pow", TYPE_FLOAT);

    return sa;
}

/* Add semantic error */
static void add_semantic_error(SemanticAnalyzer *sa, int line, const char *format, ...) {
    char msg[512];
    va_list args;
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);

    char full_msg[600];
    snprintf(full_msg, sizeof(full_msg), "ERROR (line %d): %s\n", line, msg);
    strcat(sa->errors, full_msg);
    sa->error_count++;
}

/* Add semantic warning */
static void add_semantic_warning(SemanticAnalyzer *sa, int line, const char *format, ...) {
    char msg[512];
    va_list args;
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);

    char full_msg[600];
    snprintf(full_msg, sizeof(full_msg), "WARNING (line %d): %s\n", line, msg);
    strcat(sa->warnings, full_msg);
    sa->warning_count++;
}

/* Enter new scope */
static void enter_scope(SemanticAnalyzer *sa, const char *name) {
    if (sa->scope_count >= MAX_SCOPES) {
        fprintf(stderr, "Scope stack overflow\n");
        return;
    }
    sa->current_scope++;
    sa->scope_stack[sa->scope_count] = sa->current_scope;
    strcpy(sa->scope_names[sa->scope_count], name);
    sa->scope_count++;
    printf("  -> Entering scope: %s (level %d)\n", name, sa->current_scope);
}

/* Exit current scope */
static void exit_scope(SemanticAnalyzer *sa) {
    if (sa->scope_count <= 1) return;

    printf("  <- Exiting scope: %s (level %d)\n",
           sa->scope_names[sa->scope_count - 1], sa->current_scope);

    sa->scope_count--;
    sa->current_scope = sa->scope_stack[sa->scope_count - 1];
}

/* Declare symbol */
static void declare_symbol(SemanticAnalyzer *sa, const char *name, DataType type,
                           int line, int is_function, int is_array) {
    /* Check for redeclaration at same scope */
    for (int i = 0; i < sa->symbols->count; i++) {
        Symbol *s = &sa->symbols->symbols[i];
        if (strcmp(s->name, name) == 0 && s->scope_level == sa->current_scope) {
            add_semantic_error(sa, line, "Redeclaration of '%s'", name);
            return;
        }
    }

    Symbol *sym = &sa->symbols->symbols[sa->symbols->count++];
    strcpy(sym->name, name);
    sym->type = type;
    strcpy(sym->type_name, data_type_to_string(type));
    sym->scope_level = sa->current_scope;
    sym->is_function = is_function;
    sym->is_array = is_array;
    sym->line_declared = line;
    strcpy(sym->scope_name, sa->scope_names[sa->scope_count - 1]);

    printf("  + Declared: %s [%s] at scope %d\n", name, sym->type_name, sa->current_scope);
}

/* Lookup symbol */
static Symbol* lookup(SemanticAnalyzer *sa, const char *name) {
    /* Search from current scope outward */
    for (int scope = sa->current_scope; scope >= 0; scope--) {
        for (int i = sa->symbols->count - 1; i >= 0; i--) {
            Symbol *s = &sa->symbols->symbols[i];
            if (strcmp(s->name, name) == 0 && s->scope_level <= scope) {
                return s;
            }
        }
    }
    return NULL;
}

/* Check type compatibility */
static int types_compatible(DataType t1, DataType t2) {
    if (t1 == t2) return 1;
    if (t1 == TYPE_UNKNOWN || t2 == TYPE_UNKNOWN) return 1;

    /* int and float are compatible (implicit conversion) */
    if ((t1 == TYPE_INT && t2 == TYPE_FLOAT) ||
        (t1 == TYPE_FLOAT && t2 == TYPE_INT)) {
        return 1;
    }

    return 0;
}

/* Get promoted type */
static DataType promote_types(DataType t1, DataType t2) {
    if (t1 == TYPE_FLOAT || t2 == TYPE_FLOAT) return TYPE_FLOAT;
    return TYPE_INT;
}

/* Analyze expression and return its type */
static DataType analyze_expression(SemanticAnalyzer *sa, ASTNode *node) {
    if (!node) return TYPE_UNKNOWN;

    switch (node->type) {
        case AST_INT_LITERAL:
            return TYPE_INT;

        case AST_FLOAT_LITERAL:
            return TYPE_FLOAT;

        case AST_IDENTIFIER: {
            Symbol *sym = lookup(sa, node->value);
            if (!sym) {
                add_semantic_error(sa, node->line,
                    "Undeclared variable '%s'", node->value);
                return TYPE_UNKNOWN;
            }
            return sym->type;
        }

        case AST_BINARY_OP: {
            if (node->child_count < 2) return TYPE_UNKNOWN;

            DataType left = analyze_expression(sa, node->children[0]);
            DataType right = analyze_expression(sa, node->children[1]);

            /* Logical operators return bool */
            if (strcmp(node->value, "&&") == 0 ||
                strcmp(node->value, "||") == 0 ||
                strcmp(node->value, "==") == 0 ||
                strcmp(node->value, "!=") == 0 ||
                strcmp(node->value, "<") == 0 ||
                strcmp(node->value, "<=") == 0 ||
                strcmp(node->value, ">") == 0 ||
                strcmp(node->value, ">=") == 0) {
                return TYPE_BOOL;
            }

            /* Arithmetic operators */
            if (!types_compatible(left, right)) {
                add_semantic_error(sa, node->line,
                    "Type mismatch in binary operation: %s %s %s",
                    data_type_to_string(left), node->value, data_type_to_string(right));
            }

            return promote_types(left, right);
        }

        case AST_UNARY_OP: {
            if (node->child_count < 1) return TYPE_UNKNOWN;
            DataType operand = analyze_expression(sa, node->children[0]);

            if (strcmp(node->value, "!") == 0) {
                return TYPE_BOOL;
            }
            return operand;
        }

        case AST_CALL: {
            Symbol *func = lookup(sa, node->value);
            if (!func) {
                add_semantic_error(sa, node->line,
                    "Undeclared function '%s'", node->value);
                return TYPE_UNKNOWN;
            }
            if (!func->is_function) {
                add_semantic_error(sa, node->line,
                    "'%s' is not a function", node->value);
                return TYPE_UNKNOWN;
            }

            /* Analyze arguments */
            for (int i = 0; i < node->child_count; i++) {
                analyze_expression(sa, node->children[i]);
            }

            return func->type;
        }

        case AST_ARRAY_ACCESS: {
            Symbol *sym = lookup(sa, node->value);
            if (!sym) {
                add_semantic_error(sa, node->line,
                    "Undeclared array '%s'", node->value);
                return TYPE_UNKNOWN;
            }
            if (!sym->is_array) {
                add_semantic_error(sa, node->line,
                    "'%s' is not an array", node->value);
            }

            /* Check index type */
            if (node->child_count > 0) {
                DataType idx_type = analyze_expression(sa, node->children[0]);
                if (idx_type != TYPE_INT && idx_type != TYPE_UNKNOWN) {
                    add_semantic_error(sa, node->line,
                        "Array index must be integer");
                }
            }

            return sym->type;
        }

        case AST_MATH_FUNC: {
            /* log, exp, sqrt, sin, cos, tan all take float and return float */
            if (node->child_count > 0) {
                DataType arg = analyze_expression(sa, node->children[0]);
                if (arg != TYPE_INT && arg != TYPE_FLOAT && arg != TYPE_UNKNOWN) {
                    add_semantic_error(sa, node->line,
                        "Math function '%s' requires numeric argument", node->value);
                }
            }
            return TYPE_FLOAT;
        }

        default:
            return TYPE_UNKNOWN;
    }
}

/* Analyze variable declaration */
static void analyze_var_decl(SemanticAnalyzer *sa, ASTNode *node) {
    DataType type = string_to_data_type(node->data_type);
    int is_array = 0;

    /* Check for array */
    if (node->child_count > 0 && node->children[0]->type == AST_INT_LITERAL) {
        is_array = 1;
    }

    declare_symbol(sa, node->value, type, node->line, 0, is_array);

    /* Check initializer type */
    int init_idx = is_array ? 1 : 0;
    if (node->child_count > init_idx) {
        DataType init_type = analyze_expression(sa, node->children[init_idx]);
        if (!types_compatible(type, init_type)) {
            add_semantic_error(sa, node->line,
                "Type mismatch in initialization: cannot assign %s to %s",
                data_type_to_string(init_type), data_type_to_string(type));
        } else if (type == TYPE_INT && init_type == TYPE_FLOAT) {
            add_semantic_warning(sa, node->line,
                "Implicit conversion from float to int may lose precision");
        }
    }
}

/* Analyze assignment */
static void analyze_assignment(SemanticAnalyzer *sa, ASTNode *node) {
    Symbol *sym = lookup(sa, node->value);
    if (!sym) {
        add_semantic_error(sa, node->line,
            "Assignment to undeclared variable '%s'", node->value);
        return;
    }

    /* Get value expression (last child) */
    int value_idx = node->child_count - 1;
    if (value_idx < 0) return;

    DataType value_type = analyze_expression(sa, node->children[value_idx]);

    if (!types_compatible(sym->type, value_type)) {
        add_semantic_error(sa, node->line,
            "Type mismatch in assignment: cannot assign %s to %s",
            data_type_to_string(value_type), data_type_to_string(sym->type));
    } else if (sym->type == TYPE_INT && value_type == TYPE_FLOAT) {
        add_semantic_warning(sa, node->line,
            "Implicit conversion from float to int");
    }
}

/* Analyze if statement */
static void analyze_if(SemanticAnalyzer *sa, ASTNode *node) {
    /* Condition should be boolean or convertible */
    if (node->child_count > 0) {
        DataType cond_type = analyze_expression(sa, node->children[0]);
        if (cond_type != TYPE_BOOL && cond_type != TYPE_INT && cond_type != TYPE_UNKNOWN) {
            add_semantic_warning(sa, node->line,
                "Condition expression should be boolean");
        }
    }

    /* Then block */
    if (node->child_count > 1) {
        analyze_block(sa, node->children[1]);
    }

    /* Else block */
    if (node->child_count > 2) {
        analyze_block(sa, node->children[2]);
    }
}

/* Analyze while statement */
static void analyze_while(SemanticAnalyzer *sa, ASTNode *node) {
    /* Condition */
    if (node->child_count > 0) {
        DataType cond_type = analyze_expression(sa, node->children[0]);
        if (cond_type != TYPE_BOOL && cond_type != TYPE_INT && cond_type != TYPE_UNKNOWN) {
            add_semantic_warning(sa, node->line,
                "Loop condition should be boolean");
        }
    }

    /* Body */
    if (node->child_count > 1) {
        enter_scope(sa, "while_body");
        analyze_block(sa, node->children[1]);
        exit_scope(sa);
    }
}

/* Analyze for statement */
static void analyze_for(SemanticAnalyzer *sa, ASTNode *node) {
    enter_scope(sa, "for_body");

    /* Init */
    if (node->child_count > 0) {
        analyze_statement(sa, node->children[0]);
    }

    /* Condition */
    if (node->child_count > 1) {
        analyze_expression(sa, node->children[1]);
    }

    /* Update */
    if (node->child_count > 2) {
        analyze_expression(sa, node->children[2]);
    }

    /* Body */
    if (node->child_count > 3) {
        analyze_block(sa, node->children[3]);
    }

    exit_scope(sa);
}

/* Analyze return statement */
static void analyze_return(SemanticAnalyzer *sa, ASTNode *node) {
    if (node->child_count > 0) {
        analyze_expression(sa, node->children[0]);
    }
}

/* Analyze statement */
static void analyze_statement(SemanticAnalyzer *sa, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_VAR_DECL:
            analyze_var_decl(sa, node);
            break;
        case AST_ASSIGNMENT:
            analyze_assignment(sa, node);
            break;
        case AST_IF_STMT:
            analyze_if(sa, node);
            break;
        case AST_WHILE_STMT:
            analyze_while(sa, node);
            break;
        case AST_FOR_STMT:
            analyze_for(sa, node);
            break;
        case AST_RETURN_STMT:
            analyze_return(sa, node);
            break;
        case AST_BLOCK:
            analyze_block(sa, node);
            break;
        case AST_CALL:
        case AST_BINARY_OP:
        case AST_UNARY_OP:
            analyze_expression(sa, node);
            break;
        default:
            break;
    }
}

/* Analyze block */
static void analyze_block(SemanticAnalyzer *sa, ASTNode *node) {
    if (!node) return;

    for (int i = 0; i < node->child_count; i++) {
        analyze_statement(sa, node->children[i]);
    }
}

/* Analyze function */
static void analyze_function(SemanticAnalyzer *sa, ASTNode *node) {
    DataType return_type = string_to_data_type(node->data_type);

    /* Declare function */
    declare_symbol(sa, node->value, return_type, node->line, 1, 0);

    /* Enter function scope */
    enter_scope(sa, node->value);

    /* Declare parameters */
    if (node->child_count > 0 && node->children[0]->type == AST_PARAM_LIST) {
        ASTNode *params = node->children[0];
        for (int i = 0; i < params->child_count; i++) {
            ASTNode *param = params->children[i];
            DataType param_type = string_to_data_type(param->data_type);
            declare_symbol(sa, param->value, param_type, param->line, 0, 0);
        }
    }

    /* Analyze body */
    if (node->child_count > 1) {
        analyze_block(sa, node->children[1]);
    }

    exit_scope(sa);
}

/* Analyze program node */
static void analyze_node(SemanticAnalyzer *sa, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->child_count; i++) {
                analyze_node(sa, node->children[i]);
            }
            break;
        case AST_FUNCTION:
            analyze_function(sa, node);
            break;
        default:
            analyze_statement(sa, node);
            break;
    }
}

/* Main semantic analysis function */
int semantic_analyze(ASTNode *ast, SymbolTable **out_symbols,
                     int *error_count, int *warning_count,
                     char *errors, char *warnings) {
    SemanticAnalyzer *sa = create_semantic_analyzer();

    printf("\n--- Semantic Analysis ---\n");
    analyze_node(sa, ast);

    /* Copy results */
    if (out_symbols) {
        *out_symbols = sa->symbols;
    }
    if (error_count) *error_count = sa->error_count;
    if (warning_count) *warning_count = sa->warning_count;
    if (errors) strcpy(errors, sa->errors);
    if (warnings) strcpy(warnings, sa->warnings);

    int result = (sa->error_count == 0) ? 0 : -1;

    /* Don't free symbols if we're returning them */
    if (!out_symbols) {
        free_symbol_table(sa->symbols);
    }
    free(sa);

    return result;
}

/* Print semantic analysis report */
void print_semantic_report(int errors, int warnings,
                           const char *error_msg, const char *warning_msg) {
    printf("\n========== SEMANTIC ANALYSIS REPORT ==========\n");
    printf("Errors: %d, Warnings: %d\n\n", errors, warnings);

    if (strlen(error_msg) > 0) {
        printf("--- Errors ---\n%s\n", error_msg);
    }
    if (strlen(warning_msg) > 0) {
        printf("--- Warnings ---\n%s\n", warning_msg);
    }
    if (errors == 0 && warnings == 0) {
        printf("No errors or warnings.\n");
    }
    printf("==============================================\n");
}

/* Standalone main for testing */
#ifdef SEMANTIC_STANDALONE
int main(int argc, char *argv[]) {
    char source[100000];
    int len = 0;

    printf("=== MODULE 5: SEMANTIC ANALYSIS ===\n\n");

    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
        len = fread(source, 1, sizeof(source) - 1, f);
        source[len] = '\0';
        fclose(f);
        printf("Source file: %s\n", argv[1]);
    } else {
        printf("Reading from stdin...\n");
        len = fread(source, 1, sizeof(source) - 1, stdin);
        source[len] = '\0';
    }

    printf("\n--- Source Code ---\n%s\n", source);

    /* Tokenize */
    extern TokenStream* tokenize(const char *source);
    TokenStream *tokens = tokenize(source);

    /* Parse */
    extern ASTNode* parse(TokenStream *tokens, int *error_count, char *error_msg);
    int parse_errors;
    char parse_error_msg[5000];
    ASTNode *ast = parse(tokens, &parse_errors, parse_error_msg);

    if (parse_errors > 0) {
        printf("\n--- Parse Errors ---\n%s", parse_error_msg);
        return 1;
    }

    /* Semantic analysis */
    SymbolTable *symbols;
    int sem_errors, sem_warnings;
    char errors[10000], warnings[5000];

    semantic_analyze(ast, &symbols, &sem_errors, &sem_warnings, errors, warnings);

    print_symbol_table(symbols);
    print_semantic_report(sem_errors, sem_warnings, errors, warnings);

    free_symbol_table(symbols);
    free_ast(ast);
    free(tokens);

    return sem_errors > 0 ? 1 : 0;
}
#endif
