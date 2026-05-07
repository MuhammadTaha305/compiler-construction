=== MEMBER 1 — LEXER + PARSER + EXTENDED GRAMMAR ===

DEPENDENCIES:
  - flex (or win_flex on Windows)
  - bison (or win_bison on Windows)
  - gcc
  - libm (math library, linked with -lm)

BUILD ALL:
  make member1

RUN INDIVIDUAL TESTS:
  make test_lexer            → runs lexer on lexer/test_input.txt
  make test_postfix_eval     → runs postfix stack evaluator
  make test_postfix_parser   → runs bison postfix parser
  make test_prefix_parser    → runs prefix parser (type expressions manually)
  make test_infix_parser     → runs infix parser on parser/infix_test.txt
  make test_extended_parser  → runs extended parser on parser/extended_test.txt

WINDOWS (no make):
  win_bison -d parser/infix.y
  win_flex parser/infix_lex.l
  gcc -o infix_parser parser/infix.tab.c lex.yy.c -lm

FILES:
  lexer/lexer.l            - Main tokeniser (Deliverable 1)
  lexer/postfix.l          - Postfix stack evaluator (Lab 05 Task 2)
  parser/postfix.y + postfix_lex.l  - Postfix parser (Deliverable 3)
  parser/prefix.y  + prefix_lex.l   - Prefix parser  (Deliverable 3)
  parser/infix.y   + infix_lex.l    - Infix parser   (Deliverable 3)
  parser/extended.y + extended_lex.l - Extended grammar log/exp/^ (Deliverable 4)