%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define YYDEBUG 1

void yyerror(const char *s);
int yylex();
%}

%union {
    double dval;
    char  *sval;
}

%token <dval> INT_LIT FLOAT_LIT
%token <sval> ID
%token LOG EXP
%token PLUS MINUS TIMES DIVIDE POWER
%token LPAREN RPAREN

%type <dval> expr term factor base

%%

input:
    /* empty */
    | input line
    ;

line:
    expr '\n'  { printf("==================\nRESULT = %g\n==================\n", $1); }
    | '\n'
    | error '\n' { yyerrok; printf("Recovered from error, enter new expression.\n"); }
    ;

expr:
    expr PLUS  term   { $$ = $1 + $3; printf("  => %g + %g = %g\n", $1, $3, $$); }
  | expr MINUS term   { $$ = $1 - $3; printf("  => %g - %g = %g\n", $1, $3, $$); }
  | term              { $$ = $1; }
  ;

term:
    term TIMES  factor  { $$ = $1 * $3; printf("  => %g * %g = %g\n", $1, $3, $$); }
  | term DIVIDE factor  {
        if ($3 == 0) { yyerror("Division by zero"); $$ = 0; }
        else { $$ = $1 / $3; printf("  => %g / %g = %g\n", $1, $3, $$); }
    }
  | factor              { $$ = $1; }
  ;

factor:
    base POWER factor   { $$ = pow($1, $3); printf("  => %g ^ %g = %g\n", $1, $3, $$); }
  | base                { $$ = $1; }
  ;

base:
    LPAREN expr RPAREN              { $$ = $2; }
  | LOG LPAREN expr RPAREN         { $$ = log($3); printf("  => log(%g) = %g\n", $3, $$); }
  | EXP LPAREN expr RPAREN         { $$ = exp($3); printf("  => exp(%g) = %g\n", $3, $$); }
  | INT_LIT                        { $$ = $1; }
  | FLOAT_LIT                      { $$ = $1; }
  | ID                             { $$ = 0; printf("  WARN: variable '%s' not resolved, using 0\n", $1); free($1); }
  ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
        yyin = f;
    }
    yydebug = 0; /* set to 1 to enable debug output */
    printf("=== EXTENDED PARSER (log / exp / ^ support) ===\n");
    yyparse();
    return 0;
}