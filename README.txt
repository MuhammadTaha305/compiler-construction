================================================================================
                    MINI-COMPILER PROJECT
                 CS-346 Compiler Construction
================================================================================

PROJECT OVERVIEW
----------------
This project implements a complete mini-compiler with 8 phases:
  1. Lexical Analysis (Tokenization)
  2. Syntax Analysis (Parsing)
  3. Extended Grammar (Functions, Math)
  4. Grammar Analysis (FIRST/FOLLOW/LL(1))
  5. Semantic Analysis (Type & Scope Checking)
  6. IR Generation (Three-Address Code)
  7. Code Optimization (Constant Folding, CSE, Dead Code)
  8. LLVM IR Generation

DIRECTORY STRUCTURE
-------------------
project/
  lexer/           - Module 1: Lexical Analysis (.l files)
  parser/          - Module 2 & 3: Syntax Analysis (.y files)
  first_follow/    - Module 4: FIRST/FOLLOW/LL(1) programs
  semantic/        - Module 5: Type & Scope checking
  ir/              - Module 6: Three-Address Code generation
  optimizer/       - Module 7: Code optimization passes
  llvm/            - Module 8: LLVM IR generation
  common/          - Shared headers and utilities
  makefile         - Build configuration
  main.c           - Integrated compiler entry point
  README.txt       - This file

BUILD INSTRUCTIONS
------------------
Prerequisites:
  - GCC compiler
  - Flex (win_flex on Windows)
  - Bison (win_bison on Windows)
  - LLVM/Clang (optional, for Module 8)

Building the integrated compiler:
  $ make all
  or simply:
  $ make

Building individual modules:
  $ make module1    # Lexer
  $ make module2    # Parsers (postfix, prefix, infix, extended)
  $ make module4    # FIRST/FOLLOW/LL(1)
  $ make module5    # Semantic Analysis
  $ make module6    # IR Generation
  $ make module7    # Optimization
  $ make module8    # LLVM Generation

Building all module tests:
  $ make modules

RUNNING THE COMPILER
--------------------
Basic usage:
  $ ./compiler <source_file>

With options:
  $ ./compiler -o output.txt program.c
  $ ./compiler -v program.c          # Verbose mode
  $ ./compiler -h                    # Show help

Examples:
  $ ./compiler test_input.c
  $ ./compiler complex_test.c -o result.txt

Without input file (uses built-in test):
  $ ./compiler

OUTPUT FILES
------------
- pipeline_output.txt    Complete compilation log
- ir/output.tac          Three-Address Code
- llvm/output.ll         LLVM IR code

TESTING
-------
Run all tests:
  $ make test_all

Individual module tests:
  $ make test_lexer       # Test tokenization
  $ make test_parser      # Test parsing
  $ make test_semantic    # Test semantic analysis
  $ make test_tac         # Test TAC generation
  $ make test_optimizer   # Test optimization
  $ make test_llvm        # Test LLVM generation

SAMPLE INPUT
------------
// test_input.c
int main() {
    int a = 5;
    int b = 10;
    int c = a + b;
    if (c > 10) {
        c = c * 2;
    }
    return c;
}

SUPPORTED LANGUAGE FEATURES
---------------------------
Types:
  - int, float, char, bool, void
  - Arrays

Operators:
  - Arithmetic: + - * / % ^
  - Comparison: < <= > >= == !=
  - Logical: && || !
  - Assignment: =

Statements:
  - Variable declarations
  - Assignments
  - If-else
  - While loops
  - For loops
  - Function definitions
  - Return statements

Math Functions:
  - log(), exp(), sqrt()
  - sin(), cos(), tan()

OPTIMIZATION TECHNIQUES
-----------------------
Module 7 implements the following optimizations:
  - Constant Folding
  - Constant Propagation
  - Common Subexpression Elimination (CSE)
  - Dead Code Elimination
  - Loop-Invariant Code Motion

CLEANING
--------
Remove all generated files:
  $ make clean

HELP
----
Show all make targets:
  $ make help

================================================================================
