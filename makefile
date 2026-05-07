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

# ─── Clean ──────────────────────────────────────────────────────────
clean:
	rm -f lex.yy.c *.tab.c *.tab.h
	rm -f lexer/lexer lexer/postfix_eval
	rm -f parser/postfix_parser parser/prefix_parser parser/infix_parser parser/extended_parser
	rm -f parser/*.tab.c parser/*.tab.h parser/*.yy.c
	@echo "Cleaned."