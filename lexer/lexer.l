%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations */
void yyerror(const char *s);
%}

%option yylineno
%option noyywrap

%%

"if"        { printf("[KEYWORD: if]\n");        }
"then"      { printf("[KEYWORD: then]\n");      }
"begin"     { printf("[KEYWORD: begin]\n");     }
"end"       { printf("[KEYWORD: end]\n");       }
"procedure" { printf("[KEYWORD: procedure]\n"); }
"function"  { printf("[KEYWORD: function]\n");  }

[0-9]+\.[0-9]+  { printf("[FLOAT_LIT: %s]\n", yytext); }
[0-9]+          { printf("[INT_LIT: %s]\n",   yytext); }

[a-z][a-zA-Z0-9]*  { printf("[ID: %s]\n", yytext); }

"+"   { printf("[OP: +]\n");  }
"-"   { printf("[OP: -]\n");  }
"*"   { printf("[OP: *]\n");  }
"/"   { printf("[OP: /]\n");  }

[ \t\n\r]+  { /* discard whitespace */ }

.  { printf("ERROR: unrecognised character '%s' at line %d\n", yytext, yylineno); }

%%

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Cannot open file: %s\n", argv[1]);
            return 1;
        }
        yyin = f;
    }
    printf("=== LEXER TOKEN OUTPUT ===\n");
    yylex();
    printf("=== END OF TOKEN STREAM ===\n");
    return 0;
}