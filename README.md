# Mini C Compiler — 8-Phase Pipeline

> 📚 **Course:** Compiler Construction — Spring 2026, NUST SEECS

An integrated mini-compiler for a C-like language, written in C with Flex and Bison. Source code is processed through eight sequential phases — from tokenisation to **LLVM IR generation** — and the project ships with a web-based **Compiler Visualization Dashboard** that shows the output of each phase.

> CS-346 Compiler Construction project. ~20k LOC across the phases.

## Compilation pipeline

| Phase | Stage | Tooling | Directory |
|---|---|---|---|
| 1 | Lexical Analysis (tokenisation) | Flex | `lexer/` |
| 2 | Syntax Analysis (parsing → AST) | Bison | `parser/` |
| 3 | Extended Grammar Support | Bison | `parser/` |
| 4 | Grammar Analysis (FIRST/FOLLOW, LL(1) table) | C | `first_follow/` |
| 5 | Semantic Analysis (type & scope checking, symbol table) | C | `semantic/` |
| 6 | IR Generation (Three-Address Code) | C | `ir/` |
| 7 | Code Optimisation | C | `optimizer/` |
| 8 | LLVM IR Generation | C | `llvm/` |

Shared utilities live in `common/`. A standalone C HTTP server in `web/` serves the visualization dashboard.

## Build

Requires `gcc`, `flex`, and `bison` (`win_flex` / `win_bison` on Windows).

```bash
make            # builds the integrated `compiler` binary
```

## Run

```bash
./compiler <input_file> <output_file>

# examples
./compiler test_input.c    pipeline_output.txt   # simple program
./compiler complex_test.c  complex_output.txt    # advanced program
```

The output file contains the result of every phase: token stream, AST, FIRST/FOLLOW sets and LL(1) table, the type-checked symbol table, the generated Three-Address Code, the optimised IR, and the final LLVM IR.

## Web dashboard

```bash
cd web
# build and run server.c (see web/ for platform notes), then open index.html
```

The dashboard ("Compiler Visualization Dashboard") lets you paste source and inspect each compilation phase interactively.

## Repository layout

```
.
├── makefile             # builds the integrated `compiler` binary (all 8 phases)
├── main.c               # driver — runs the full pipeline
├── test_input.c         # simple test program
├── complex_test.c       # advanced test program
├── lexer/               # Phase 1  — Flex lexer
├── parser/              # Phases 2–3 — Bison grammar (infix/prefix/postfix/extended)
├── first_follow/        # Phase 4  — FIRST/FOLLOW/LL(1) analysis
├── semantic/            # Phase 5  — type & scope checking
├── ir/                  # Phase 6  — Three-Address Code generation
├── optimizer/           # Phase 7  — IR optimisation
├── llvm/                # Phase 8  — LLVM IR backend
├── common/              # shared headers/utilities
└── web/                 # C HTTP server + visualization dashboard
```

## Authors

Built collaboratively by:
- **[Muhammad Taha](https://github.com/MuhammadTaha305)**
- **[Sham](https://github.com/shamtarani05)**
- **[Abdul Moiz](https://github.com/AbdulMoiz132)**
- **[Maier Ali](https://github.com/atomicfalcon)**
