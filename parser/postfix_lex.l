%{
#include <stdio.h>
#include <stdlib.h>
#include "postfix.tab.h"
%}

%option noyywrap
%option yylineno

%%

[0-9]+      { yylval.ival = atoi(yytext); return INT_LIT; }
[+\-*]      { return yytext[0]; }
"\n"        { return '\n'; }
[ \t]+      { /* skip */ }
.           { printf("ERROR: unrecognised character '%s'\n", yytext); }

%%