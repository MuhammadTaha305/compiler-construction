
# INTEGRATED C COMPILER

PROJECT OVERVIEW:
  This is a fully functional integrated C compiler that processes source code
  through 6 compilation phases sequentially, producing Three-Address Code (TAC)
  as intermediate representation.

QUICK START:
  1. Compile:  gcc -o integrated_compiler integrated_compiler.c -lm
  2. Run:      ./integrated_compiler test_input.c pipeline_output.txt
  3. View:     type pipeline_output.txt

COMPILATION PIPELINE (6 PHASES)

  [1] LEXICAL ANALYSIS (Module 1 - Flex)
      Input:  Source Code (.c file)
      Output: Token Stream (keywords, identifiers, operators, literals)

  [2] SYNTAX ANALYSIS / PARSING (Module 2 - Bison)
      Input:  Token Stream
      Output: Abstract Syntax Tree (AST)

  [3] EXTENDED GRAMMAR SUPPORT (Module 3)
      Input:  AST
      Output: Enhanced AST (math functions, advanced operators)

  [4] GRAMMAR ANALYSIS (Module 4)
      Input:  Grammar Rules
      Output: FIRST/FOLLOW Sets, LL(1) Parsing Table, Conflict Report

  [5] SEMANTIC ANALYSIS (Module 5)
      Input:  AST from Parsing
      Output: Type-checked & Scope-verified AST, Symbol Table

  [6] INTERMEDIATE REPRESENTATION (Module 6 - TAC)
      Input:  Validated AST
      Output: Three-Address Code (Ready for optimization)


## FILES STRUCTURE


INTEGRATED COMPILER (MAIN):
  integrated_compiler.c    - Main unified driver (200+ lines)
  test_input.c             - Simple test program
  complex_test.c           - Advanced test program

INDIVIDUAL MODULES (EXISTING):
  lexer/                   - Module 1 (Lexical Analysis with Flex)
  parser/                  - Modules 2-3 (Parsing with Bison, Extended Grammar)
  module4/                 - Module 4 (FIRST/FOLLOW/LL(1) Analysis)
  module5/                 - Module 5 (Semantic Analysis & Type Checking)
  module6/                 - Module 6 (TAC Code Generation)

DOCUMENTATION:
  INDEX.md                           - Master index and overview
  QUICK_START.md                     - Quick reference guide
  INTEGRATED_COMPILER_GUIDE.md       - Complete usage guide
  INTEGRATED_COMPILER_STATUS.md      - Detailed implementation report
  PROJECT_COMPLETION_SUMMARY.md      - Full project summary

OUTPUT FILES:
  pipeline_output.txt      - Output from test_input.c compilation
  complex_output.txt       - Output from complex_test.c compilation
  final_test.txt           - Latest verification test output


## BUILDING THE COMPILER

WINDOWS (using gcc with MinGW):
  gcc -o integrated_compiler integrated_compiler.c -lm

LINUX/MAC:
  gcc -o integrated_compiler integrated_compiler.c -lm

VERIFY BUILD:
  ./integrated_compiler --help  (displays usage)


## RUNNING THE COMPILER

BASIC USAGE:
  ./integrated_compiler <input_file> <output_file>

EXAMPLES:

  Test 1 - Simple Program:
    ./integrated_compiler test_input.c pipeline_output.txt

  Test 2 - Complex Program:
    ./integrated_compiler complex_test.c complex_output.txt

  Test 3 - Custom Program:
    ./integrated_compiler my_program.c my_output.txt

VIEW OUTPUT:
  type pipeline_output.txt        (Windows)
  cat pipeline_output.txt         (Linux/Mac)



