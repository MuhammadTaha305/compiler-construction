# =============================================================================
# MINI-COMPILER PROJECT - MAKEFILE
# CS-346 Compiler Construction
# =============================================================================
# Directory Structure (as per guidelines):
#   lexer/        - Module 1: Lexical Analysis
#   parser/       - Module 2 & 3: Syntax Analysis, Extended Grammar
#   first_follow/ - Module 4: FIRST/FOLLOW/LL(1)
#   semantic/     - Module 5: Type & Scope Checking
#   ir/           - Module 6: IR Generation (TAC)
#   optimizer/    - Module 7: Code Optimization
#   llvm/         - Module 8: LLVM IR Generation
#   common/       - Shared headers and utilities
# =============================================================================

CC = gcc
CFLAGS = -Wall -g -I.
LDFLAGS = -lm

# Platform detection for Windows
ifeq ($(OS),Windows_NT)
    EXE = .exe
    RM = del /Q 2>nul
    FLEX = win_flex
    BISON = win_bison
else
    EXE =
    RM = rm -f
    FLEX = flex
    BISON = bison
endif

# =============================================================================
# MAIN INTEGRATED COMPILER (All 8 Phases)
# =============================================================================

COMMON_SRC = common/common.c
LEXER_SRC = lexer/lexer_integrated.c
PARSER_SRC = parser/parser_integrated.c
SEMANTIC_SRC = semantic/semantic_integrated.c
IR_SRC = ir/ir_generator.c
OPTIMIZER_SRC = optimizer/optimizer.c
LLVM_SRC = llvm/llvm_generator.c
MAIN_SRC = main.c

ALL_SRC = $(COMMON_SRC) $(LEXER_SRC) $(PARSER_SRC) $(SEMANTIC_SRC) \
          $(IR_SRC) $(OPTIMIZER_SRC) $(LLVM_SRC) $(MAIN_SRC)

# Default target: Build integrated compiler
all: compiler$(EXE)
	@echo ""
	@echo "================================================"
	@echo "  Integrated Compiler Built Successfully!"
	@echo "  Run: ./compiler$(EXE) <source_file>"
	@echo "================================================"

compiler$(EXE): $(ALL_SRC)
	$(CC) $(CFLAGS) -o $@ $(ALL_SRC) $(LDFLAGS)
	@echo "Build OK: compiler$(EXE)"

# =============================================================================
# MODULE 1: LEXICAL ANALYSIS
# =============================================================================

# Original Flex-based lexer
lexer: lexer/lexer.l
	cd lexer && $(FLEX) lexer.l
	$(CC) -o lexer/lexer$(EXE) lexer/lex.yy.c -lfl
	@echo "Build OK: lexer/lexer$(EXE)"

# Integrated lexer (standalone test)
lexer_integrated: $(COMMON_SRC) lexer/lexer_integrated.c
	$(CC) $(CFLAGS) -DLEXER_STANDALONE -o lexer/lexer_test$(EXE) $(COMMON_SRC) lexer/lexer_integrated.c $(LDFLAGS)
	@echo "Build OK: lexer/lexer_test$(EXE)"

# Postfix evaluator
postfix_eval: lexer/postfix.l
	cd lexer && $(FLEX) postfix.l
	$(CC) -o lexer/postfix_eval$(EXE) lexer/lex.yy.c -lfl
	@echo "Build OK: lexer/postfix_eval$(EXE)"

# Module 1 all targets
module1: lexer_integrated
	@echo "Module 1 (Lexer) built."

# =============================================================================
# MODULE 2 & 3: SYNTAX ANALYSIS (Parsers)
# =============================================================================

postfix_parser:
	cd parser && $(BISON) -d postfix.y -o postfix.tab.c
	cd parser && $(FLEX) -o postfix_lex.yy.c postfix_lex.l
	$(CC) -o parser/postfix_parser$(EXE) parser/postfix.tab.c parser/postfix_lex.yy.c -lfl -lm
	@echo "Build OK: parser/postfix_parser$(EXE)"

prefix_parser:
	cd parser && $(BISON) -d prefix.y -o prefix.tab.c
	cd parser && $(FLEX) -o prefix_lex.yy.c prefix_lex.l
	$(CC) -o parser/prefix_parser$(EXE) parser/prefix.tab.c parser/prefix_lex.yy.c -lfl -lm
	@echo "Build OK: parser/prefix_parser$(EXE)"

infix_parser:
	cd parser && $(BISON) -d infix.y -o infix.tab.c
	cd parser && $(FLEX) -o infix_lex.yy.c infix_lex.l
	$(CC) -o parser/infix_parser$(EXE) parser/infix.tab.c parser/infix_lex.yy.c -lfl -lm
	@echo "Build OK: parser/infix_parser$(EXE)"

extended_parser:
	cd parser && $(BISON) -d extended.y -o extended.tab.c
	cd parser && $(FLEX) -o extended_lex.yy.c extended_lex.l
	$(CC) -o parser/extended_parser$(EXE) parser/extended.tab.c parser/extended_lex.yy.c -lfl -lm
	@echo "Build OK: parser/extended_parser$(EXE)"

# Integrated parser (standalone test)
parser_integrated: $(COMMON_SRC) lexer/lexer_integrated.c parser/parser_integrated.c
	$(CC) $(CFLAGS) -DPARSER_STANDALONE -o parser/parser_test$(EXE) $(COMMON_SRC) lexer/lexer_integrated.c parser/parser_integrated.c $(LDFLAGS)
	@echo "Build OK: parser/parser_test$(EXE)"

# Module 2 & 3 all targets
module2: postfix_parser prefix_parser infix_parser extended_parser parser_integrated
	@echo "Module 2 & 3 (Parsers) built."

module3: module2
	@echo "Module 3 (Extended Grammar) included in module2."

# =============================================================================
# MODULE 4: FIRST & FOLLOW / LL(1)
# =============================================================================

first_sets:
	$(CC) $(CFLAGS) -o first_follow/first_sets$(EXE) first_follow/grammar.c first_follow/first_follow.c first_follow/first_sets.c $(LDFLAGS)
	@echo "Build OK: first_follow/first_sets$(EXE)"

follow_sets:
	$(CC) $(CFLAGS) -o first_follow/follow_sets$(EXE) first_follow/grammar.c first_follow/first_follow.c first_follow/follow_sets.c $(LDFLAGS)
	@echo "Build OK: first_follow/follow_sets$(EXE)"

ll1_table:
	$(CC) $(CFLAGS) -o first_follow/ll1_table$(EXE) first_follow/grammar.c first_follow/first_follow.c first_follow/ll1_table.c first_follow/ll1_table_main.c $(LDFLAGS)
	@echo "Build OK: first_follow/ll1_table$(EXE)"

module4: first_sets follow_sets ll1_table
	@echo "Module 4 (FIRST/FOLLOW/LL(1)) built."

# =============================================================================
# MODULE 5: SEMANTIC ANALYSIS
# =============================================================================

semantic_test:
	$(CC) $(CFLAGS) -o semantic/semantic_test$(EXE) semantic/semantic.c semantic/semantic_test.c $(LDFLAGS)
	@echo "Build OK: semantic/semantic_test$(EXE)"

semantic_integrated: $(COMMON_SRC) lexer/lexer_integrated.c parser/parser_integrated.c semantic/semantic_integrated.c
	$(CC) $(CFLAGS) -DSEMANTIC_STANDALONE -o semantic/semantic_integrated_test$(EXE) $(COMMON_SRC) lexer/lexer_integrated.c parser/parser_integrated.c semantic/semantic_integrated.c $(LDFLAGS)
	@echo "Build OK: semantic/semantic_integrated_test$(EXE)"

module5: semantic_test semantic_integrated
	@echo "Module 5 (Semantic Analysis) built."

# =============================================================================
# MODULE 6: IR GENERATION (TAC)
# =============================================================================

tac_test:
	$(CC) $(CFLAGS) -o ir/tac_test$(EXE) ir/tac.c ir/tac_test.c $(LDFLAGS)
	@echo "Build OK: ir/tac_test$(EXE)"

ir_integrated: $(COMMON_SRC) lexer/lexer_integrated.c parser/parser_integrated.c semantic/semantic_integrated.c ir/ir_generator.c
	$(CC) $(CFLAGS) -DIR_STANDALONE -o ir/ir_test$(EXE) $(COMMON_SRC) lexer/lexer_integrated.c parser/parser_integrated.c semantic/semantic_integrated.c ir/ir_generator.c $(LDFLAGS)
	@echo "Build OK: ir/ir_test$(EXE)"

module6: tac_test ir_integrated
	@echo "Module 6 (IR Generation) built."

# =============================================================================
# MODULE 7: CODE OPTIMIZATION
# =============================================================================

optimizer_test: $(COMMON_SRC) optimizer/optimizer.c
	$(CC) $(CFLAGS) -DOPTIMIZER_STANDALONE -o optimizer/optimizer_test$(EXE) $(COMMON_SRC) optimizer/optimizer.c $(LDFLAGS)
	@echo "Build OK: optimizer/optimizer_test$(EXE)"

module7: optimizer_test
	@echo "Module 7 (Optimization) built."

# =============================================================================
# MODULE 8: LLVM IR GENERATION
# =============================================================================

llvm_test: $(COMMON_SRC) llvm/llvm_generator.c
	$(CC) $(CFLAGS) -DLLVM_STANDALONE -o llvm/llvm_test$(EXE) $(COMMON_SRC) llvm/llvm_generator.c $(LDFLAGS)
	@echo "Build OK: llvm/llvm_test$(EXE)"

module8: llvm_test
	@echo "Module 8 (LLVM Generation) built."

# =============================================================================
# WEB SERVER (Dashboard Backend)
# =============================================================================

SERVER_SRC = web/server.c $(COMMON_SRC) $(LEXER_SRC) $(PARSER_SRC) $(SEMANTIC_SRC) $(IR_SRC)

server$(EXE): $(SERVER_SRC)
ifeq ($(OS),Windows_NT)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC) -lws2_32 $(LDFLAGS)
else
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC) $(LDFLAGS)
endif
	@echo ""
	@echo "================================================"
	@echo "  Web Server Built Successfully!"
	@echo "  Run: ./server$(EXE)"
	@echo "  Then open http://localhost:8080"
	@echo "================================================"

# =============================================================================
# BUILD ALL MODULES
# =============================================================================

modules: module1 module2 module4 module5 module6 module7 module8
	@echo ""
	@echo "================================================"
	@echo "  All Modules Built Successfully!"
	@echo "================================================"

# Legacy targets for compatibility
member1: module1 module2
	@echo "Member 1 targets built."

member2: module4 module5 module6
	@echo "Member 2 targets built."

# =============================================================================
# TEST TARGETS
# =============================================================================

test_compiler: compiler$(EXE)
	./compiler$(EXE) test_input.c
	@echo "Compiler test complete."

test_lexer: lexer_integrated
	./lexer/lexer_test$(EXE) test_input.c

test_parser: parser_integrated
	./parser/parser_test$(EXE) test_input.c

test_infix: infix_parser
	./parser/infix_parser$(EXE) < parser/infix_test.txt

test_extended: extended_parser
	./parser/extended_parser$(EXE) < parser/extended_test.txt

test_first_sets: first_sets
	./first_follow/first_sets$(EXE) first_follow/grammar.txt

test_follow_sets: follow_sets
	./first_follow/follow_sets$(EXE) first_follow/grammar.txt

test_ll1_table: ll1_table
	./first_follow/ll1_table$(EXE) first_follow/grammar.txt

test_semantic: semantic_test
	./semantic/semantic_test$(EXE)

test_tac: tac_test
	./ir/tac_test$(EXE)

test_optimizer: optimizer_test
	./optimizer/optimizer_test$(EXE)

test_llvm: llvm_test
	./llvm/llvm_test$(EXE)

test_all: test_compiler test_optimizer test_llvm
	@echo ""
	@echo "================================================"
	@echo "  All Tests Completed!"
	@echo "================================================"

# =============================================================================
# CLEAN
# =============================================================================

clean:
	-$(RM) compiler$(EXE)
	-$(RM) server$(EXE)
	-$(RM) lex.yy.c *.tab.c *.tab.h
	-$(RM) lexer\lexer$(EXE) lexer\lexer_test$(EXE) lexer\postfix_eval$(EXE)
	-$(RM) lexer\lex.yy.c
	-$(RM) parser\postfix_parser$(EXE) parser\prefix_parser$(EXE)
	-$(RM) parser\infix_parser$(EXE) parser\extended_parser$(EXE)
	-$(RM) parser\parser_test$(EXE)
	-$(RM) parser\*.tab.c parser\*.tab.h parser\*.yy.c
	-$(RM) first_follow\first_sets$(EXE) first_follow\follow_sets$(EXE)
	-$(RM) first_follow\ll1_table$(EXE) first_follow\*.csv
	-$(RM) semantic\semantic_test$(EXE) semantic\semantic_integrated_test$(EXE)
	-$(RM) ir\tac_test$(EXE) ir\ir_test$(EXE) ir\*.tac
	-$(RM) optimizer\optimizer_test$(EXE)
	-$(RM) llvm\llvm_test$(EXE) llvm\*.ll
	-$(RM) pipeline_output.txt
	@echo "Cleaned."

# =============================================================================
# HELP
# =============================================================================

help:
	@echo ""
	@echo "=============================================="
	@echo "  Mini-Compiler Project - Build System"
	@echo "=============================================="
	@echo ""
	@echo "MAIN TARGETS:"
	@echo "  make              Build integrated 8-phase compiler"
	@echo "  make all          Same as above"
	@echo "  make server       Build web dashboard server"
	@echo "  make modules      Build all module tests"
	@echo ""
	@echo "MODULE TARGETS:"
	@echo "  make module1      Lexer (lexer/)"
	@echo "  make module2      Parsers (parser/)"
	@echo "  make module4      FIRST/FOLLOW/LL(1) (first_follow/)"
	@echo "  make module5      Semantic Analysis (semantic/)"
	@echo "  make module6      IR Generation (ir/)"
	@echo "  make module7      Optimization (optimizer/)"
	@echo "  make module8      LLVM Generation (llvm/)"
	@echo ""
	@echo "WEB DASHBOARD:"
	@echo "  make server       Build web server"
	@echo "  ./server.exe      Run server (open http://localhost:8080)"
	@echo ""
	@echo "TEST TARGETS:"
	@echo "  make test_compiler   Test full pipeline"
	@echo "  make test_lexer      Test tokenization"
	@echo "  make test_parser     Test parsing"
	@echo "  make test_optimizer  Test optimization"
	@echo "  make test_llvm       Test LLVM generation"
	@echo "  make test_all        Run all tests"
	@echo ""
	@echo "UTILITY:"
	@echo "  make clean        Remove generated files"
	@echo "  make help         Show this help"
	@echo ""

.PHONY: all clean help modules test_all server
.PHONY: module1 module2 module3 module4 module5 module6 module7 module8
.PHONY: member1 member2
.PHONY: lexer lexer_integrated postfix_eval
.PHONY: postfix_parser prefix_parser infix_parser extended_parser parser_integrated
.PHONY: first_sets follow_sets ll1_table
.PHONY: semantic_test semantic_integrated
.PHONY: tac_test ir_integrated
.PHONY: optimizer_test llvm_test
