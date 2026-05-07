%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extended.tab.h"
%}

%option noyywrap
%option yylineno

%%

"//".*                         { /* discard single-line comment */ }
"/*"([^*]|\*+[^*/])*\*+"/"   { /* discard multi-line comment */ }

"log"   { return LOG; }
"exp"   { return EXP; }

[0-9]+\.[0-9]+  { yylval.dval = atof(yytext); return FLOAT_LIT; }
[0-9]+          { yylval.dval = atof(yytext); return INT_LIT;   }

[a-zA-Z_][a-zA-Z0-9_]*  { yylval.sval = strdup(yytext); return ID; }

"+"   { return PLUS;   }
"-"   { return MINUS;  }
"*"   { return TIMES;  }
"/"   { return DIVIDE; }
"^"   { return POWER;  }
"("   { return LPAREN; }
")"   { return RPAREN; }
"\n"  { return '\n';   }

[ \t]+  { /* skip whitespace */ }

.  { printf("ERROR: unrecognised character '%s' at line %d\n", yytext, yylineno); }

%%