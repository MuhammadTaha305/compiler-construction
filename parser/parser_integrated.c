/*
 * MODULE 2 & 3: SYNTAX ANALYSIS (Integrated Parser)
 * Mini-Compiler Project - CS-346 Compiler Construction
 *
 * Recursive descent parser with AST generation.
 * Supports extended grammar with functions, exponentiation, and math functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/common.h"

/* Parser state */
typedef struct {
    TokenStream *tokens;
    int pos;
    int error_count;
    char error_msg[5000];
} Parser;

/* Forward declarations */
static ASTNode* parse_program(Parser *p);
static ASTNode* parse_function(Parser *p);
static ASTNode* parse_block(Parser *p);
static ASTNode* parse_statement(Parser *p);
static ASTNode* parse_declaration(Parser *p);
static ASTNode* parse_assignment(Parser *p);
static ASTNode* parse_if_statement(Parser *p);
static ASTNode* parse_while_statement(Parser *p);
static ASTNode* parse_for_statement(Parser *p);
static ASTNode* parse_return_statement(Parser *p);
static ASTNode* parse_expression(Parser *p);
static ASTNode* parse_logical_or(Parser *p);
static ASTNode* parse_logical_and(Parser *p);
static ASTNode* parse_equality(Parser *p);
static ASTNode* parse_relational(Parser *p);
static ASTNode* parse_additive(Parser *p);
static ASTNode* parse_multiplicative(Parser *p);
static ASTNode* parse_power(Parser *p);
static ASTNode* parse_unary(Parser *p);
static ASTNode* parse_primary(Parser *p);
static ASTNode* parse_function_call(Parser *p, const char *name);

/* Get current token */
static Token* current(Parser *p) {
    if (p->pos >= p->tokens->count) {
        return &p->tokens->tokens[p->tokens->count - 1];  /* EOF */
    }
    return &p->tokens->tokens[p->pos];
}

/* Peek ahead */
static Token* peek(Parser *p, int ahead) {
    int idx = p->pos + ahead;
    if (idx >= p->tokens->count) {
        return &p->tokens->tokens[p->tokens->count - 1];
    }
    return &p->tokens->tokens[idx];
}

/* Advance to next token */
static Token* advance(Parser *p) {
    if (p->pos < p->tokens->count) {
        return &p->tokens->tokens[p->pos++];
    }
    return &p->tokens->tokens[p->tokens->count - 1];
}

/* Check current token type */
static int check(Parser *p, MiniTokenType type) {
    return current(p)->type == type;
}

/* Match and consume token */
static int match(Parser *p, MiniTokenType type) {
    if (check(p, type)) {
        advance(p);
        return 1;
    }
    return 0;
}

/* Expect token or error */
static Token* expect(Parser *p, MiniTokenType type, const char *msg) {
    if (check(p, type)) {
        return advance(p);
    }
    char err[256];
    snprintf(err, sizeof(err), "Line %d: Expected %s but found '%s'\n",
             current(p)->line, msg, current(p)->lexeme);
    strcat(p->error_msg, err);
    p->error_count++;
    return NULL;
}

/* Check if token is a type keyword */
static int is_type_keyword(MiniTokenType type) {
    return type == TOK_INT || type == TOK_FLOAT ||
           type == TOK_CHAR || type == TOK_VOID || type == TOK_BOOL;
}

/* Create parser */
Parser* create_parser(TokenStream *tokens) {
    Parser *p = (Parser *)malloc(sizeof(Parser));
    p->tokens = tokens;
    p->pos = 0;
    p->error_count = 0;
    p->error_msg[0] = '\0';
    return p;
}

/* Parse program: list of functions and global declarations */
static ASTNode* parse_program(Parser *p) {
    ASTNode *program = create_ast_node(AST_PROGRAM, "program");

    while (!check(p, TOK_EOF)) {
        ASTNode *node = NULL;

        if (is_type_keyword(current(p)->type)) {
            /* Could be function or variable declaration */
            if (peek(p, 1)->type == TOK_IDENTIFIER &&
                peek(p, 2)->type == TOK_LPAREN) {
                node = parse_function(p);
            } else {
                node = parse_declaration(p);
            }
        } else if (check(p, TOK_FUNCTION) || check(p, TOK_PROCEDURE)) {
            node = parse_function(p);
        } else {
            node = parse_statement(p);
        }

        if (node) {
            add_ast_child(program, node);
        } else {
            /* Skip error token */
            advance(p);
        }
    }

    return program;
}

/* Parse function definition */
static ASTNode* parse_function(Parser *p) {
    char type_name[32] = "void";

    /* Optional type */
    if (is_type_keyword(current(p)->type)) {
        strcpy(type_name, current(p)->lexeme);
        advance(p);
    } else if (match(p, TOK_FUNCTION) || match(p, TOK_PROCEDURE)) {
        /* Already consumed keyword */
    }

    Token *name_tok = expect(p, TOK_IDENTIFIER, "function name");
    if (!name_tok) return NULL;

    ASTNode *func = create_ast_node(AST_FUNCTION, name_tok->lexeme);
    strcpy(func->data_type, type_name);
    func->line = name_tok->line;

    expect(p, TOK_LPAREN, "(");

    /* Parse parameters */
    ASTNode *params = create_ast_node(AST_PARAM_LIST, "params");
    while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF)) {
        if (is_type_keyword(current(p)->type)) {
            char param_type[32];
            strcpy(param_type, current(p)->lexeme);
            advance(p);

            Token *param_name = expect(p, TOK_IDENTIFIER, "parameter name");
            if (param_name) {
                ASTNode *param = create_ast_node(AST_PARAM, param_name->lexeme);
                strcpy(param->data_type, param_type);
                add_ast_child(params, param);
            }
        }

        if (!match(p, TOK_COMMA)) break;
    }
    add_ast_child(func, params);

    expect(p, TOK_RPAREN, ")");

    /* Parse body */
    ASTNode *body = parse_block(p);
    if (body) {
        add_ast_child(func, body);
    }

    return func;
}

/* Parse block { ... } */
static ASTNode* parse_block(Parser *p) {
    ASTNode *block = create_ast_node(AST_BLOCK, "block");

    if (match(p, TOK_LBRACE) || match(p, TOK_BEGIN)) {
        while (!check(p, TOK_RBRACE) && !check(p, TOK_END) && !check(p, TOK_EOF)) {
            ASTNode *stmt = parse_statement(p);
            if (stmt) {
                add_ast_child(block, stmt);
            }
        }
        if (!match(p, TOK_RBRACE)) {
            match(p, TOK_END);
        }
    } else {
        /* Single statement */
        ASTNode *stmt = parse_statement(p);
        if (stmt) {
            add_ast_child(block, stmt);
        }
    }

    return block;
}

/* Parse statement */
static ASTNode* parse_statement(Parser *p) {
    if (check(p, TOK_EOF)) return NULL;

    /* Declaration */
    if (is_type_keyword(current(p)->type)) {
        return parse_declaration(p);
    }

    /* If statement */
    if (check(p, TOK_IF)) {
        return parse_if_statement(p);
    }

    /* While statement */
    if (check(p, TOK_WHILE)) {
        return parse_while_statement(p);
    }

    /* For statement */
    if (check(p, TOK_FOR)) {
        return parse_for_statement(p);
    }

    /* Return statement */
    if (check(p, TOK_RETURN)) {
        return parse_return_statement(p);
    }

    /* Block */
    if (check(p, TOK_LBRACE) || check(p, TOK_BEGIN)) {
        return parse_block(p);
    }

    /* Assignment or expression statement */
    if (check(p, TOK_IDENTIFIER)) {
        if (peek(p, 1)->type == TOK_ASSIGN ||
            peek(p, 1)->type == TOK_LBRACKET) {
            return parse_assignment(p);
        }
    }

    /* Expression statement */
    ASTNode *expr = parse_expression(p);
    match(p, TOK_SEMICOLON);
    return expr;
}

/* Parse declaration: type id [= expr] ; */
static ASTNode* parse_declaration(Parser *p) {
    char type_name[32];
    strcpy(type_name, current(p)->lexeme);
    advance(p);

    Token *name_tok = expect(p, TOK_IDENTIFIER, "variable name");
    if (!name_tok) return NULL;

    ASTNode *decl = create_ast_node(AST_VAR_DECL, name_tok->lexeme);
    strcpy(decl->data_type, type_name);
    decl->line = name_tok->line;

    /* Array declaration */
    if (match(p, TOK_LBRACKET)) {
        if (check(p, TOK_INT_LIT)) {
            ASTNode *size = create_ast_node(AST_INT_LITERAL, current(p)->lexeme);
            add_ast_child(decl, size);
            advance(p);
        }
        expect(p, TOK_RBRACKET, "]");
    }

    /* Initialization */
    if (match(p, TOK_ASSIGN)) {
        ASTNode *init = parse_expression(p);
        if (init) {
            add_ast_child(decl, init);
        }
    }

    match(p, TOK_SEMICOLON);
    return decl;
}

/* Parse assignment: id = expr ; */
static ASTNode* parse_assignment(Parser *p) {
    Token *name_tok = advance(p);
    ASTNode *assign = create_ast_node(AST_ASSIGNMENT, name_tok->lexeme);
    assign->line = name_tok->line;

    /* Array index */
    if (match(p, TOK_LBRACKET)) {
        ASTNode *index = parse_expression(p);
        if (index) {
            add_ast_child(assign, index);
        }
        expect(p, TOK_RBRACKET, "]");
    }

    expect(p, TOK_ASSIGN, "=");

    ASTNode *value = parse_expression(p);
    if (value) {
        add_ast_child(assign, value);
    }

    match(p, TOK_SEMICOLON);
    return assign;
}

/* Parse if statement */
static ASTNode* parse_if_statement(Parser *p) {
    Token *if_tok = advance(p);
    ASTNode *if_stmt = create_ast_node(AST_IF_STMT, "if");
    if_stmt->line = if_tok->line;

    expect(p, TOK_LPAREN, "(");
    ASTNode *cond = parse_expression(p);
    if (cond) add_ast_child(if_stmt, cond);
    expect(p, TOK_RPAREN, ")");

    match(p, TOK_THEN);  /* Optional 'then' */

    ASTNode *then_block = parse_block(p);
    if (then_block) add_ast_child(if_stmt, then_block);

    if (match(p, TOK_ELSE)) {
        ASTNode *else_block = parse_block(p);
        if (else_block) add_ast_child(if_stmt, else_block);
    }

    return if_stmt;
}

/* Parse while statement */
static ASTNode* parse_while_statement(Parser *p) {
    Token *while_tok = advance(p);
    ASTNode *while_stmt = create_ast_node(AST_WHILE_STMT, "while");
    while_stmt->line = while_tok->line;

    expect(p, TOK_LPAREN, "(");
    ASTNode *cond = parse_expression(p);
    if (cond) add_ast_child(while_stmt, cond);
    expect(p, TOK_RPAREN, ")");

    match(p, TOK_DO);  /* Optional 'do' */

    ASTNode *body = parse_block(p);
    if (body) add_ast_child(while_stmt, body);

    return while_stmt;
}

/* Parse for statement */
static ASTNode* parse_for_statement(Parser *p) {
    Token *for_tok = advance(p);
    ASTNode *for_stmt = create_ast_node(AST_FOR_STMT, "for");
    for_stmt->line = for_tok->line;

    expect(p, TOK_LPAREN, "(");

    /* Init */
    if (!check(p, TOK_SEMICOLON)) {
        ASTNode *init = parse_statement(p);
        if (init) add_ast_child(for_stmt, init);
    } else {
        add_ast_child(for_stmt, create_ast_node(AST_INT_LITERAL, "0"));
        advance(p);
    }

    /* Condition */
    if (!check(p, TOK_SEMICOLON)) {
        ASTNode *cond = parse_expression(p);
        if (cond) add_ast_child(for_stmt, cond);
    } else {
        add_ast_child(for_stmt, create_ast_node(AST_INT_LITERAL, "1"));
    }
    expect(p, TOK_SEMICOLON, ";");

    /* Update */
    if (!check(p, TOK_RPAREN)) {
        ASTNode *update = parse_expression(p);
        if (update) add_ast_child(for_stmt, update);
    }
    expect(p, TOK_RPAREN, ")");

    /* Body */
    ASTNode *body = parse_block(p);
    if (body) add_ast_child(for_stmt, body);

    return for_stmt;
}

/* Parse return statement */
static ASTNode* parse_return_statement(Parser *p) {
    Token *ret_tok = advance(p);
    ASTNode *ret_stmt = create_ast_node(AST_RETURN_STMT, "return");
    ret_stmt->line = ret_tok->line;

    if (!check(p, TOK_SEMICOLON)) {
        ASTNode *value = parse_expression(p);
        if (value) add_ast_child(ret_stmt, value);
    }

    match(p, TOK_SEMICOLON);
    return ret_stmt;
}

/* Parse expression (entry point) */
static ASTNode* parse_expression(Parser *p) {
    return parse_logical_or(p);
}

/* Parse logical OR: expr || expr */
static ASTNode* parse_logical_or(Parser *p) {
    ASTNode *left = parse_logical_and(p);

    while (match(p, TOK_OR)) {
        ASTNode *op = create_ast_node(AST_BINARY_OP, "||");
        ASTNode *right = parse_logical_and(p);
        add_ast_child(op, left);
        add_ast_child(op, right);
        left = op;
    }

    return left;
}

/* Parse logical AND: expr && expr */
static ASTNode* parse_logical_and(Parser *p) {
    ASTNode *left = parse_equality(p);

    while (match(p, TOK_AND)) {
        ASTNode *op = create_ast_node(AST_BINARY_OP, "&&");
        ASTNode *right = parse_equality(p);
        add_ast_child(op, left);
        add_ast_child(op, right);
        left = op;
    }

    return left;
}

/* Parse equality: expr == expr | expr != expr */
static ASTNode* parse_equality(Parser *p) {
    ASTNode *left = parse_relational(p);

    while (check(p, TOK_EQ) || check(p, TOK_NE)) {
        Token *op_tok = advance(p);
        ASTNode *op = create_ast_node(AST_BINARY_OP, op_tok->lexeme);
        ASTNode *right = parse_relational(p);
        add_ast_child(op, left);
        add_ast_child(op, right);
        left = op;
    }

    return left;
}

/* Parse relational: expr < expr | expr <= expr | ... */
static ASTNode* parse_relational(Parser *p) {
    ASTNode *left = parse_additive(p);

    while (check(p, TOK_LT) || check(p, TOK_LE) ||
           check(p, TOK_GT) || check(p, TOK_GE)) {
        Token *op_tok = advance(p);
        ASTNode *op = create_ast_node(AST_BINARY_OP, op_tok->lexeme);
        ASTNode *right = parse_additive(p);
        add_ast_child(op, left);
        add_ast_child(op, right);
        left = op;
    }

    return left;
}

/* Parse additive: expr + expr | expr - expr */
static ASTNode* parse_additive(Parser *p) {
    ASTNode *left = parse_multiplicative(p);

    while (check(p, TOK_PLUS) || check(p, TOK_MINUS)) {
        Token *op_tok = advance(p);
        ASTNode *op = create_ast_node(AST_BINARY_OP, op_tok->lexeme);
        ASTNode *right = parse_multiplicative(p);
        add_ast_child(op, left);
        add_ast_child(op, right);
        left = op;
    }

    return left;
}

/* Parse multiplicative: expr * expr | expr / expr | expr % expr */
static ASTNode* parse_multiplicative(Parser *p) {
    ASTNode *left = parse_power(p);

    while (check(p, TOK_STAR) || check(p, TOK_SLASH) || check(p, TOK_MOD)) {
        Token *op_tok = advance(p);
        ASTNode *op = create_ast_node(AST_BINARY_OP, op_tok->lexeme);
        ASTNode *right = parse_power(p);
        add_ast_child(op, left);
        add_ast_child(op, right);
        left = op;
    }

    return left;
}

/* Parse power: base ^ exponent (right-associative) */
static ASTNode* parse_power(Parser *p) {
    ASTNode *base = parse_unary(p);

    if (match(p, TOK_POWER)) {
        ASTNode *op = create_ast_node(AST_BINARY_OP, "^");
        ASTNode *exp = parse_power(p);  /* Right-associative recursion */
        add_ast_child(op, base);
        add_ast_child(op, exp);
        return op;
    }

    return base;
}

/* Parse unary: -expr | !expr | primary */
static ASTNode* parse_unary(Parser *p) {
    if (check(p, TOK_MINUS) || check(p, TOK_NOT)) {
        Token *op_tok = advance(p);
        ASTNode *op = create_ast_node(AST_UNARY_OP, op_tok->lexeme);
        ASTNode *operand = parse_unary(p);
        add_ast_child(op, operand);
        return op;
    }

    return parse_primary(p);
}

/* Parse primary: literal | id | (expr) | func(args) */
static ASTNode* parse_primary(Parser *p) {
    /* Integer literal */
    if (check(p, TOK_INT_LIT)) {
        Token *tok = advance(p);
        ASTNode *node = create_ast_node(AST_INT_LITERAL, tok->lexeme);
        node->line = tok->line;
        strcpy(node->data_type, "int");
        return node;
    }

    /* Float literal */
    if (check(p, TOK_FLOAT_LIT)) {
        Token *tok = advance(p);
        ASTNode *node = create_ast_node(AST_FLOAT_LITERAL, tok->lexeme);
        node->line = tok->line;
        strcpy(node->data_type, "float");
        return node;
    }

    /* Math functions: log, exp, sqrt, sin, cos, tan */
    if (check(p, TOK_LOG) || check(p, TOK_EXP) || check(p, TOK_SQRT) ||
        check(p, TOK_SIN) || check(p, TOK_COS) || check(p, TOK_TAN)) {
        Token *func_tok = advance(p);
        ASTNode *func = create_ast_node(AST_MATH_FUNC, func_tok->lexeme);
        func->line = func_tok->line;
        strcpy(func->data_type, "float");

        expect(p, TOK_LPAREN, "(");
        ASTNode *arg = parse_expression(p);
        if (arg) add_ast_child(func, arg);
        expect(p, TOK_RPAREN, ")");

        return func;
    }

    /* Identifier or function call */
    if (check(p, TOK_IDENTIFIER)) {
        Token *id_tok = advance(p);

        /* Function call */
        if (check(p, TOK_LPAREN)) {
            return parse_function_call(p, id_tok->lexeme);
        }

        /* Array access */
        if (check(p, TOK_LBRACKET)) {
            ASTNode *access = create_ast_node(AST_ARRAY_ACCESS, id_tok->lexeme);
            access->line = id_tok->line;
            advance(p);  /* [ */
            ASTNode *index = parse_expression(p);
            if (index) add_ast_child(access, index);
            expect(p, TOK_RBRACKET, "]");
            return access;
        }

        /* Simple identifier */
        ASTNode *node = create_ast_node(AST_IDENTIFIER, id_tok->lexeme);
        node->line = id_tok->line;
        return node;
    }

    /* Parenthesized expression */
    if (match(p, TOK_LPAREN)) {
        ASTNode *expr = parse_expression(p);
        expect(p, TOK_RPAREN, ")");
        return expr;
    }

    /* Error */
    char err[256];
    snprintf(err, sizeof(err), "Line %d: Unexpected token '%s'\n",
             current(p)->line, current(p)->lexeme);
    strcat(p->error_msg, err);
    p->error_count++;
    advance(p);
    return NULL;
}

/* Parse function call */
static ASTNode* parse_function_call(Parser *p, const char *name) {
    ASTNode *call = create_ast_node(AST_CALL, name);

    expect(p, TOK_LPAREN, "(");

    while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF)) {
        ASTNode *arg = parse_expression(p);
        if (arg) add_ast_child(call, arg);

        if (!match(p, TOK_COMMA)) break;
    }

    expect(p, TOK_RPAREN, ")");
    return call;
}

/* Main parse function */
ASTNode* parse(TokenStream *tokens, int *error_count, char *error_msg) {
    Parser *p = create_parser(tokens);
    ASTNode *ast = parse_program(p);

    *error_count = p->error_count;
    if (error_msg) {
        strcpy(error_msg, p->error_msg);
    }

    free(p);
    return ast;
}

/* Print AST in tree format */
void print_ast_tree(ASTNode *node, int depth, char *prefix, int is_last) {
    if (!node) return;

    printf("%s", prefix);
    printf("%s", is_last ? "+-- " : "|-- ");

    const char *type_names[] = {
        "Program", "Function", "VarDecl", "Assignment", "BinaryOp",
        "UnaryOp", "IfStmt", "WhileStmt", "ForStmt", "ReturnStmt",
        "Block", "Call", "Identifier", "IntLiteral", "FloatLiteral",
        "ArrayAccess", "MathFunc", "Param", "ParamList"
    };

    printf("%s", type_names[node->type]);
    if (node->value[0]) {
        printf(": %s", node->value);
    }
    if (node->data_type[0]) {
        printf(" [%s]", node->data_type);
    }
    printf("\n");

    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "|   ");

    for (int i = 0; i < node->child_count; i++) {
        print_ast_tree(node->children[i], depth + 1, new_prefix, i == node->child_count - 1);
    }
}

/* Standalone main for testing */
#ifdef PARSER_STANDALONE
int main(int argc, char *argv[]) {
    char source[100000];
    int len = 0;

    printf("=== MODULE 2 & 3: SYNTAX ANALYSIS ===\n\n");

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
    int error_count;
    char error_msg[5000];
    ASTNode *ast = parse(tokens, &error_count, error_msg);

    if (error_count > 0) {
        printf("\n--- Parse Errors ---\n%s", error_msg);
    }

    printf("\n--- Abstract Syntax Tree ---\n");
    print_ast_tree(ast, 0, "", 1);

    free_ast(ast);
    free(tokens);
    return error_count > 0 ? 1 : 0;
}
#endif
