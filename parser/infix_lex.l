%{
#include <stdio.h>
#include <stdlib.h>
#include "infix.tab.h"
%}

%option noyywrap
%option yylineno

%%

"//".*                         { /* discard single-line comment */ }
"/*"([^*]|\*+[^*/])*\*+"/"   { /* discard multi-line comment */ }

[0-9]+\.[0-9]+  { yylval.dval = atof(yytext); return FLOAT_LIT; }
[0-9]+          { yylval.dval = atof(yytext); return INT_LIT;   }

"+"   { return PLUS;   }
"-"   { return MINUS;  }
"*"   { return TIMES;  }
"/"   { return DIVIDE; }
"^"   { return POWER;  }
"("   { return '(';    }
")"   { return ')';    }
"\n"  { return '\n';   }

[ \t]+  { /* skip whitespace */ }

.  { printf("ERROR: unrecognised character '%s' at line %d\n", yytext, yylineno); }

%%