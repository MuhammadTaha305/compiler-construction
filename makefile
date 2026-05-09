CC      = gcc
CFLAGS  = -Wall -lfl -lm

# ─── Lexer standalone ───────────────────────────────────────────────
lexer:
	flex lexer/lexer.l
	$(CC) -o lexer/lexer lex.yy.c -lfl
	@echo "Build OK: lexer/lexer"

# ─── Postfix evaluator (flex only) ─────────────────────────────────
postfix_eval:
	flex lexer/postfix.l
	$(CC) -o lexer/postfix_eval lex.yy.c -lfl
	@echo "Build OK: lexer/postfix_eval"

# ─── Postfix parser (bison + flex) ──────────────────────────────────
postfix_parser:
	bison -d parser/postfix.y -o parser/postfix.tab.c
	flex -o parser/postfix_lex.yy.c parser/postfix_lex.l
	$(CC) -o parser/postfix_parser parser/postfix.tab.c parser/postfix_lex.yy.c -lfl -lm
	@echo "Build OK: parser/postfix_parser"

# ─── Prefix parser ──────────────────────────────────────────────────
prefix_parser:
	bison -d parser/prefix.y -o parser/prefix.tab.c
	flex -o parser/prefix_lex.yy.c parser/prefix_lex.l
	$(CC) -o parser/prefix_parser parser/prefix.tab.c parser/prefix_lex.yy.c -lfl -lm
	@echo "Build OK: parser/prefix_parser"

# ─── Infix parser ───────────────────────────────────────────────────
infix_parser:
	bison -d parser/infix.y -o parser/infix.tab.c
	flex -o parser/infix_lex.yy.c parser/infix_lex.l
	$(CC) -o parser/infix_parser parser/infix.tab.c parser/infix_lex.yy.c -lfl -lm
	@echo "Build OK: parser/infix_parser"

# ─── Extended parser (log/exp/^) ────────────────────────────────────
extended_parser:
	bison -d parser/extended.y -o parser/extended.tab.c
	flex -o parser/extended_lex.yy.c parser/extended_lex.l
	$(CC) -o parser/extended_parser parser/extended.tab.c parser/extended_lex.yy.c -lfl -lm
	@echo "Build OK: parser/extended_parser"

# ─── Build all Member 1 targets ────────────────────────────────────
member1: lexer postfix_eval postfix_parser prefix_parser infix_parser extended_parser
	@echo "All Member 1 targets built."

# ─── Module 4: FIRST & FOLLOW Sets / LL(1) Parsing Table ────────────
first_sets:
	$(CC) -o module4/first_sets module4/grammar.c module4/first_follow.c module4/first_sets.c -lm
	@echo "Build OK: module4/first_sets"

follow_sets:
	$(CC) -o module4/follow_sets module4/grammar.c module4/first_follow.c module4/follow_sets.c -lm
	@echo "Build OK: module4/follow_sets"

ll1_table:
	$(CC) -o module4/ll1_table module4/grammar.c module4/first_follow.c module4/ll1_table.c module4/ll1_table_main.c -lm
	@echo "Build OK: module4/ll1_table"

# ─── Module 5: Semantic Analysis (Type & Scope Checking) ────────────
semantic_test:
	$(CC) -o module5/semantic_test module5/semantic.c module5/semantic_test.c -lm
	@echo "Build OK: module5/semantic_test"

# ─── Module 6: Three-Address Code (TAC) Generation ──────────────────
tac_test:
	$(CC) -o module6/tac_test module6/tac.c module6/tac_test.c -lm
	@echo "Build OK: module6/tac_test"

# ─── Build all modules ───────────────────────────────────────────────
member2: first_sets follow_sets ll1_table semantic_test tac_test
	@echo "All Member 2 targets built (Modules 4, 5, 6)."

# ─── Run tests ──────────────────────────────────────────────────────
test_lexer:
	./lexer/lexer < lexer/test_input.txt

test_postfix_eval:
	./lexer/postfix_eval < lexer/postfix_input.txt

test_postfix_parser:
	./parser/postfix_parser < lexer/postfix_input.txt

test_prefix_parser:
	./parser/prefix_parser

test_infix_parser:
	./parser/infix_parser < parser/infix_test.txt

test_extended_parser:
	./parser/extended_parser < parser/extended_test.txt

# ─── Module 4 tests ─────────────────────────────────────────────────
test_first_sets:
	./module4/first_sets module4/grammar.txt

test_follow_sets:
	./module4/follow_sets module4/grammar.txt

test_ll1_table:
	./module4/ll1_table module4/grammar.txt module4/ll1_table_output.csv

# ─── Module 5 tests ─────────────────────────────────────────────────
test_semantic:
	./module5/semantic_test

# ─── Module 6 tests ─────────────────────────────────────────────────
test_tac:
	./module6/tac_test

# ─── Clean ──────────────────────────────────────────────────────────
clean:
	rm -f lex.yy.c *.tab.c *.tab.h
	rm -f lexer/lexer lexer/postfix_eval
	rm -f parser/postfix_parser parser/prefix_parser parser/infix_parser parser/extended_parser
	rm -f parser/*.tab.c parser/*.tab.h parser/*.yy.c
	rm -f module4/first_sets module4/follow_sets module4/ll1_table module4/*.csv
	rm -f module5/semantic_test module5/*.txt
	rm -f module6/tac_test module6/*.txt
	@echo "Cleaned."