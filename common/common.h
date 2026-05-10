/*
 * COMMON HEADER - Shared Data Structures for All Modules
 * Mini-Compiler Project - CS-346 Compiler Construction
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================
 * CONSTANTS
 * ============================================================ */
#define MAX_TOKEN_LEN 256
#define MAX_TOKENS 10000
#define MAX_SYMBOLS 500
#define MAX_SCOPES 50
#define MAX_TAC_INSTRUCTIONS 10000
#define MAX_OPERAND_LEN 100
#define MAX_LABEL_LEN 50
#define MAX_AST_NODES 5000
#define MAX_LINE_LEN 1024

/* ============================================================
 * TOKEN TYPES (Module 1 - Lexer)
 * ============================================================ */
typedef enum {
    /* Keywords */
    TOK_INT, TOK_FLOAT, TOK_CHAR, TOK_VOID, TOK_BOOL,
    TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FOR, TOK_RETURN,
    TOK_FUNCTION, TOK_PROCEDURE, TOK_BEGIN, TOK_END,
    TOK_THEN, TOK_DO,

    /* Literals */
    TOK_INT_LIT,
    TOK_FLOAT_LIT,
    TOK_STRING_LIT,
    TOK_CHAR_LIT,

    /* Identifiers */
    TOK_IDENTIFIER,

    /* Operators */
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_MOD,
    TOK_POWER,          /* ^ */
    TOK_ASSIGN,         /* = */
    TOK_EQ, TOK_NE,     /* == != */
    TOK_LT, TOK_LE,     /* < <= */
    TOK_GT, TOK_GE,     /* > >= */
    TOK_AND, TOK_OR, TOK_NOT,  /* && || ! */

    /* Delimiters */
    TOK_LPAREN, TOK_RPAREN,   /* ( ) */
    TOK_LBRACE, TOK_RBRACE,   /* { } */
    TOK_LBRACKET, TOK_RBRACKET, /* [ ] */
    TOK_SEMICOLON,            /* ; */
    TOK_COMMA,                /* , */
    TOK_COLON,                /* : */

    /* Math Functions */
    TOK_LOG, TOK_EXP, TOK_SQRT, TOK_SIN, TOK_COS, TOK_TAN,

    /* Special */
    TOK_EOF,
    TOK_NEWLINE,
    TOK_ERROR,
    TOK_UNKNOWN
} MiniTokenType;

/* Token structure */
typedef struct {
    MiniTokenType type;
    char lexeme[MAX_TOKEN_LEN];
    int line;
    int column;
    union {
        int int_val;
        double float_val;
    } value;
} Token;

/* Token stream */
typedef struct {
    Token tokens[MAX_TOKENS];
    int count;
    int current;
} TokenStream;

/* ============================================================
 * AST NODE TYPES (Module 2/3 - Parser)
 * ============================================================ */
typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_VAR_DECL,
    AST_ASSIGNMENT,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_RETURN_STMT,
    AST_BLOCK,
    AST_CALL,
    AST_IDENTIFIER,
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_ARRAY_ACCESS,
    AST_MATH_FUNC,
    AST_PARAM,
    AST_PARAM_LIST
} ASTNodeType;

/* AST Node structure */
typedef struct ASTNode {
    ASTNodeType type;
    char value[MAX_TOKEN_LEN];
    char data_type[32];
    int line;
    struct ASTNode *children[10];
    int child_count;
    struct ASTNode *next;  /* For linked lists like statements */
} ASTNode;

/* ============================================================
 * DATA TYPES (Module 5 - Semantic)
 * ============================================================ */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_STRING,
    TYPE_UNKNOWN
} DataType;

/* Symbol table entry */
typedef struct {
    char name[MAX_TOKEN_LEN];
    DataType type;
    char type_name[32];
    int scope_level;
    int is_function;
    int is_array;
    int array_size;
    int param_count;
    int line_declared;
    char scope_name[MAX_TOKEN_LEN];
} Symbol;

/* Symbol table */
typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
} SymbolTable;

/* Scope manager */
typedef struct {
    SymbolTable *table;
    int current_scope;
    int scope_stack[MAX_SCOPES];
    char scope_names[MAX_SCOPES][MAX_TOKEN_LEN];
    int scope_count;
} ScopeManager;

/* ============================================================
 * TAC OPERATIONS (Module 6 - IR)
 * ============================================================ */
typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_ASSIGN,
    OP_UMINUS,
    OP_LT, OP_LE, OP_GT, OP_GE, OP_EQ, OP_NE,
    OP_AND, OP_OR, OP_NOT,
    OP_ARRAY_REF,
    OP_ARRAY_ASSIGN,
    OP_GOTO,
    OP_IFGOTO,
    OP_IFFALSE,
    OP_PARAM,
    OP_CALL,
    OP_RETURN,
    OP_FUNC,
    OP_LABEL,
    OP_COPY,
    OP_NOP
} TACOpType;

/* TAC instruction */
typedef struct {
    TACOpType op;
    char result[MAX_OPERAND_LEN];
    char arg1[MAX_OPERAND_LEN];
    char arg2[MAX_OPERAND_LEN];
    char label[MAX_LABEL_LEN];
    int line_num;
    int is_leader;      /* For basic blocks */
    int block_id;       /* Basic block ID */
    int is_dead;        /* For dead code elimination */
} TACInstruction;

/* TAC code container */
typedef struct {
    TACInstruction instructions[MAX_TAC_INSTRUCTIONS];
    int count;
    int temp_count;
    int label_count;
} TACCode;

/* ============================================================
 * OPTIMIZATION STRUCTURES (Module 7)
 * ============================================================ */
typedef struct {
    int start_idx;
    int end_idx;
    int successors[10];
    int successor_count;
    int predecessors[10];
    int predecessor_count;
    int is_loop_header;
} BasicBlock;

typedef struct {
    BasicBlock blocks[1000];
    int count;
} CFG;  /* Control Flow Graph */

typedef struct {
    int constant_folding_count;
    int constant_propagation_count;
    int dead_code_count;
    int cse_count;
    int loop_optimization_count;
    int total_optimizations;
} OptimizationStats;

/* ============================================================
 * COMPILER STATE
 * ============================================================ */
typedef struct {
    /* Source */
    char *source_code;
    int source_length;
    char source_file[256];

    /* Phase 1: Lexer output */
    TokenStream tokens;

    /* Phase 2/3: Parser output */
    ASTNode *ast;

    /* Phase 5: Semantic output */
    SymbolTable *symbol_table;
    ScopeManager *scope_manager;
    int semantic_errors;
    int semantic_warnings;

    /* Phase 6: TAC output */
    TACCode *tac;

    /* Phase 7: Optimized TAC */
    TACCode *optimized_tac;
    CFG *cfg;
    OptimizationStats opt_stats;

    /* Compilation status */
    int error_count;
    int warning_count;
    char error_messages[10000];

} CompilerState;

/* ============================================================
 * FUNCTION PROTOTYPES
 * ============================================================ */

/* Token functions */
const char* token_type_to_string(MiniTokenType type);
MiniTokenType string_to_token_type(const char *str);

/* AST functions */
ASTNode* create_ast_node(ASTNodeType type, const char *value);
void add_ast_child(ASTNode *parent, ASTNode *child);
void print_ast(ASTNode *node, int indent);
void free_ast(ASTNode *node);

/* Symbol table functions */
SymbolTable* create_symbol_table(void);
void add_symbol(SymbolTable *table, const char *name, DataType type, int scope);
Symbol* lookup_symbol(SymbolTable *table, const char *name);
const char* data_type_to_string(DataType type);
DataType string_to_data_type(const char *str);
void print_symbol_table(SymbolTable *table);
void free_symbol_table(SymbolTable *table);

/* TAC functions */
TACCode* create_tac(void);
char* new_temp(TACCode *tac);
char* new_label(TACCode *tac);
void emit_tac(TACCode *tac, TACOpType op, const char *result, const char *arg1, const char *arg2);
void print_tac(TACCode *tac);
void free_tac(TACCode *tac);
const char* tac_op_to_string(TACOpType op);

/* Compiler state */
CompilerState* create_compiler_state(void);
void free_compiler_state(CompilerState *state);

#endif /* COMMON_H */
