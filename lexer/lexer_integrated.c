/*
 * MODULE 1: LEXICAL ANALYSIS (Integrated Version)
 * Mini-Compiler Project - CS-346 Compiler Construction
 *
 * This lexer can be used standalone or integrated with the pipeline.
 * It tokenizes source code and produces a token stream.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../common/common.h"

/* Lexer state */
typedef struct {
    const char *source;
    int pos;
    int line;
    int column;
    int length;
} Lexer;

/* Keywords table */
static struct {
    const char *keyword;
    MiniTokenType type;
} keywords[] = {
    {"int", TOK_INT},
    {"float", TOK_FLOAT},
    {"char", TOK_CHAR},
    {"void", TOK_VOID},
    {"bool", TOK_BOOL},
    {"if", TOK_IF},
    {"else", TOK_ELSE},
    {"while", TOK_WHILE},
    {"for", TOK_FOR},
    {"return", TOK_RETURN},
    {"function", TOK_FUNCTION},
    {"procedure", TOK_PROCEDURE},
    {"begin", TOK_BEGIN},
    {"end", TOK_END},
    {"then", TOK_THEN},
    {"do", TOK_DO},
    {"log", TOK_LOG},
    {"exp", TOK_EXP},
    {"sqrt", TOK_SQRT},
    {"sin", TOK_SIN},
    {"cos", TOK_COS},
    {"tan", TOK_TAN},
    {NULL, TOK_UNKNOWN}
};

/* Initialize lexer */
Lexer* create_lexer(const char *source) {
    Lexer *lex = (Lexer *)malloc(sizeof(Lexer));
    lex->source = source;
    lex->pos = 0;
    lex->line = 1;
    lex->column = 1;
    lex->length = strlen(source);
    return lex;
}

/* Peek current character */
static char peek(Lexer *lex) {
    if (lex->pos >= lex->length) return '\0';
    return lex->source[lex->pos];
}

/* Peek next character */
static char peek_next(Lexer *lex) {
    if (lex->pos + 1 >= lex->length) return '\0';
    return lex->source[lex->pos + 1];
}

/* Advance and return current character */
static char advance(Lexer *lex) {
    if (lex->pos >= lex->length) return '\0';
    char c = lex->source[lex->pos++];
    if (c == '\n') {
        lex->line++;
        lex->column = 1;
    } else {
        lex->column++;
    }
    return c;
}

/* Skip whitespace and comments */
static void skip_whitespace(Lexer *lex) {
    while (lex->pos < lex->length) {
        char c = peek(lex);

        /* Whitespace */
        if (isspace(c)) {
            advance(lex);
            continue;
        }

        /* Single-line comment */
        if (c == '/' && peek_next(lex) == '/') {
            advance(lex);
            advance(lex);
            while (peek(lex) != '\n' && peek(lex) != '\0') {
                advance(lex);
            }
            continue;
        }

        /* Multi-line comment */
        if (c == '/' && peek_next(lex) == '*') {
            advance(lex);
            advance(lex);
            while (!(peek(lex) == '*' && peek_next(lex) == '/') && peek(lex) != '\0') {
                advance(lex);
            }
            if (peek(lex) != '\0') {
                advance(lex);  /* * */
                advance(lex);  /* / */
            }
            continue;
        }

        break;
    }
}

/* Check if identifier is a keyword */
static MiniTokenType check_keyword(const char *lexeme) {
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strcmp(lexeme, keywords[i].keyword) == 0) {
            return keywords[i].type;
        }
    }
    return TOK_IDENTIFIER;
}

/* Read identifier or keyword */
static Token read_identifier(Lexer *lex) {
    Token tok;
    tok.line = lex->line;
    tok.column = lex->column;

    int start = lex->pos;
    while (isalnum(peek(lex)) || peek(lex) == '_') {
        advance(lex);
    }

    int len = lex->pos - start;
    if (len >= MAX_TOKEN_LEN) len = MAX_TOKEN_LEN - 1;
    strncpy(tok.lexeme, lex->source + start, len);
    tok.lexeme[len] = '\0';

    tok.type = check_keyword(tok.lexeme);
    return tok;
}

/* Read number (integer or float) */
static Token read_number(Lexer *lex) {
    Token tok;
    tok.line = lex->line;
    tok.column = lex->column;
    tok.type = TOK_INT_LIT;

    int start = lex->pos;
    while (isdigit(peek(lex))) {
        advance(lex);
    }

    /* Check for float */
    if (peek(lex) == '.' && isdigit(peek_next(lex))) {
        tok.type = TOK_FLOAT_LIT;
        advance(lex);  /* consume '.' */
        while (isdigit(peek(lex))) {
            advance(lex);
        }
    }

    int len = lex->pos - start;
    if (len >= MAX_TOKEN_LEN) len = MAX_TOKEN_LEN - 1;
    strncpy(tok.lexeme, lex->source + start, len);
    tok.lexeme[len] = '\0';

    if (tok.type == TOK_INT_LIT) {
        tok.value.int_val = atoi(tok.lexeme);
    } else {
        tok.value.float_val = atof(tok.lexeme);
    }

    return tok;
}

/* Read string literal */
static Token read_string(Lexer *lex) {
    Token tok;
    tok.line = lex->line;
    tok.column = lex->column;
    tok.type = TOK_STRING_LIT;

    advance(lex);  /* consume opening quote */
    int start = lex->pos;

    while (peek(lex) != '"' && peek(lex) != '\0' && peek(lex) != '\n') {
        if (peek(lex) == '\\') advance(lex);  /* escape sequence */
        advance(lex);
    }

    int len = lex->pos - start;
    if (len >= MAX_TOKEN_LEN) len = MAX_TOKEN_LEN - 1;
    strncpy(tok.lexeme, lex->source + start, len);
    tok.lexeme[len] = '\0';

    if (peek(lex) == '"') {
        advance(lex);  /* consume closing quote */
    } else {
        tok.type = TOK_ERROR;
    }

    return tok;
}

/* Get next token */
Token get_next_token(Lexer *lex) {
    Token tok;
    tok.lexeme[0] = '\0';

    skip_whitespace(lex);

    if (lex->pos >= lex->length) {
        tok.type = TOK_EOF;
        tok.line = lex->line;
        tok.column = lex->column;
        strcpy(tok.lexeme, "EOF");
        return tok;
    }

    tok.line = lex->line;
    tok.column = lex->column;
    char c = peek(lex);

    /* Identifiers and keywords */
    if (isalpha(c) || c == '_') {
        return read_identifier(lex);
    }

    /* Numbers */
    if (isdigit(c)) {
        return read_number(lex);
    }

    /* String literals */
    if (c == '"') {
        return read_string(lex);
    }

    /* Char literals */
    if (c == '\'') {
        advance(lex);
        tok.type = TOK_CHAR_LIT;
        if (peek(lex) == '\\') {
            advance(lex);
        }
        tok.lexeme[0] = advance(lex);
        tok.lexeme[1] = '\0';
        if (peek(lex) == '\'') advance(lex);
        return tok;
    }

    /* Operators and delimiters */
    advance(lex);
    tok.lexeme[0] = c;
    tok.lexeme[1] = '\0';

    switch (c) {
        case '+': tok.type = TOK_PLUS; break;
        case '-': tok.type = TOK_MINUS; break;
        case '*': tok.type = TOK_STAR; break;
        case '/': tok.type = TOK_SLASH; break;
        case '%': tok.type = TOK_MOD; break;
        case '^': tok.type = TOK_POWER; break;

        case '=':
            if (peek(lex) == '=') {
                advance(lex);
                tok.type = TOK_EQ;
                strcpy(tok.lexeme, "==");
            } else {
                tok.type = TOK_ASSIGN;
            }
            break;

        case '!':
            if (peek(lex) == '=') {
                advance(lex);
                tok.type = TOK_NE;
                strcpy(tok.lexeme, "!=");
            } else {
                tok.type = TOK_NOT;
            }
            break;

        case '<':
            if (peek(lex) == '=') {
                advance(lex);
                tok.type = TOK_LE;
                strcpy(tok.lexeme, "<=");
            } else {
                tok.type = TOK_LT;
            }
            break;

        case '>':
            if (peek(lex) == '=') {
                advance(lex);
                tok.type = TOK_GE;
                strcpy(tok.lexeme, ">=");
            } else {
                tok.type = TOK_GT;
            }
            break;

        case '&':
            if (peek(lex) == '&') {
                advance(lex);
                tok.type = TOK_AND;
                strcpy(tok.lexeme, "&&");
            } else {
                tok.type = TOK_ERROR;
            }
            break;

        case '|':
            if (peek(lex) == '|') {
                advance(lex);
                tok.type = TOK_OR;
                strcpy(tok.lexeme, "||");
            } else {
                tok.type = TOK_ERROR;
            }
            break;

        case '(': tok.type = TOK_LPAREN; break;
        case ')': tok.type = TOK_RPAREN; break;
        case '{': tok.type = TOK_LBRACE; break;
        case '}': tok.type = TOK_RBRACE; break;
        case '[': tok.type = TOK_LBRACKET; break;
        case ']': tok.type = TOK_RBRACKET; break;
        case ';': tok.type = TOK_SEMICOLON; break;
        case ',': tok.type = TOK_COMMA; break;
        case ':': tok.type = TOK_COLON; break;

        default:
            tok.type = TOK_ERROR;
            break;
    }

    return tok;
}

/* Tokenize entire source and return token stream */
TokenStream* tokenize(const char *source) {
    TokenStream *stream = (TokenStream *)malloc(sizeof(TokenStream));
    stream->count = 0;
    stream->current = 0;

    Lexer *lex = create_lexer(source);

    Token tok;
    do {
        tok = get_next_token(lex);
        if (stream->count < MAX_TOKENS) {
            stream->tokens[stream->count++] = tok;
        }
    } while (tok.type != TOK_EOF && tok.type != TOK_ERROR);

    free(lex);
    return stream;
}

/* Print token stream */
void print_token_stream(TokenStream *stream) {
    printf("\n========== TOKEN STREAM ==========\n");
    printf("Total Tokens: %d\n\n", stream->count);
    printf("%-6s %-4s %-4s %-15s %s\n", "Index", "Line", "Col", "Type", "Lexeme");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < stream->count; i++) {
        Token *t = &stream->tokens[i];
        printf("%-6d %-4d %-4d %-15s %s\n",
               i, t->line, t->column,
               token_type_to_string(t->type),
               t->lexeme);
    }
    printf("==================================\n");
}

/* Free token stream */
void free_token_stream(TokenStream *stream) {
    if (stream) free(stream);
}

/* Standalone main for testing */
#ifdef LEXER_STANDALONE
int main(int argc, char *argv[]) {
    char source[100000];
    int len = 0;

    printf("=== MODULE 1: LEXICAL ANALYSIS ===\n\n");

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
        /* Read from stdin */
        printf("Reading from stdin...\n");
        len = fread(source, 1, sizeof(source) - 1, stdin);
        source[len] = '\0';
    }

    printf("\n--- Source Code ---\n%s\n", source);

    TokenStream *stream = tokenize(source);
    print_token_stream(stream);

    free_token_stream(stream);
    return 0;
}
#endif
