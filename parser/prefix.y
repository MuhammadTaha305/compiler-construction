%{
#include <stdio.h>
#include <stdlib.h>

void yyerror(const char *s);
int yylex();
%}

%union {
    int ival;
}

%token <ival> INT_LIT
%type  <ival> expr

%%

input:
    /* empty */
    | input line
    ;

line:
    expr '\n'  { printf("==================\nFINAL RESULT = %d\n==================\n", $1); }
    | '\n'
    ;

expr:
    '+' expr expr  { $$ = $2 + $3; printf("  => %d + %d = %d\n", $2, $3, $$); }
  | '-' expr expr  { $$ = $2 - $3; printf("  => %d - %d = %d\n", $2, $3, $$); }
  | '*' expr expr  { $$ = $2 * $3; printf("  => %d * %d = %d\n", $2, $3, $$); }
  | INT_LIT        { $$ = $1;      printf("  READ %d\n", $1); }
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
    printf("=== PREFIX PARSER ===\n");
    yyparse();
    return 0;
}