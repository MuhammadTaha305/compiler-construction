%{
#include <stdio.h>
#include <stdlib.h>

int stack[100];
int top = -1;

void push(int val) {
    stack[++top] = val;
    printf("PUSH %d  |  Stack: [", val);
    for (int i = 0; i <= top; i++) {
        printf("%d", stack[i]);
        if (i < top) printf(", ");
    }
    printf("]\n");
}

int pop_val() {
    if (top < 0) {
        printf("ERROR: stack underflow\n");
        exit(1);
    }
    int val = stack[top--];
    printf("POP  %d  |  Stack: [", val);
    for (int i = 0; i <= top; i++) {
        printf("%d", stack[i]);
        if (i < top) printf(", ");
    }
    printf("]\n");
    return val;
}
%}

%option noyywrap
%option yylineno

%%

[0-9]+  {
    push(atoi(yytext));
}

"+"  {
    int b = pop_val();
    int a = pop_val();
    printf("  => %d + %d = %d\n", a, b, a + b);
    push(a + b);
}

"-"  {
    int b = pop_val();
    int a = pop_val();
    printf("  => %d - %d = %d\n", a, b, a - b);
    push(a - b);
}

"*"  {
    int b = pop_val();
    int a = pop_val();
    printf("  => %d * %d = %d\n", a, b, a * b);
    push(a * b);
}

"\n"  {
    if (top >= 0) {
        printf("==================\n");
        printf("FINAL RESULT = %d\n", stack[top]);
        printf("==================\n");
        top = -1;
    }
}

[ \t]+  { /* skip whitespace */ }

.  { printf("ERROR: unrecognised token '%s'\n", yytext); }

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
    printf("=== POSTFIX EVALUATOR ===\n");
    yylex();
    return 0;
}