%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void yyerror(const char *s);
int yylex();
%}

%union {
    double dval;
}

%token <dval> INT_LIT FLOAT_LIT
%type  <dval> expr

%left  PLUS MINUS
%left  TIMES DIVIDE
%right POWER
%right UMINUS

%%

input:
    /* empty */
    | input line
    ;

line:
    expr '\n'  { printf("==================\nRESULT = %g\n==================\n", $1); }
    | '\n'
    | error '\n' { yyerrok; printf("Recovered from error.\n"); }
    ;

expr:
    expr PLUS   expr  { $$ = $1 + $3; printf("  => %g + %g = %g\n", $1, $3, $$); }
  | expr MINUS  expr  { $$ = $1 - $3; printf("  => %g - %g = %g\n", $1, $3, $$); }
  | expr TIMES  expr  { $$ = $1 * $3; printf("  => %g * %g = %g\n", $1, $3, $$); }
  | expr DIVIDE expr  {
        if ($3 == 0) { yyerror("Division by zero"); $$ = 0; }
        else { $$ = $1 / $3; printf("  => %g / %g = %g\n", $1, $3, $$); }
    }
  | expr POWER  expr  { $$ = pow($1, $3); printf("  => %g ^ %g = %g\n", $1, $3, $$); }
  | MINUS expr %prec UMINUS { $$ = -$2; printf("  => -%g = %g\n", $2, $$); }
  | '(' expr ')'      { $$ = $2; }
  | INT_LIT           { $$ = $1; }
  | FLOAT_LIT         { $$ = $1; }
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
    printf("=== INFIX PARSER ===\n");
    yyparse();
    return 0;
}