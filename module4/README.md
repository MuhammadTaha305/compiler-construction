# MODULE 4: FIRST & FOLLOW SETS / LL(1) PARSING TABLE

## Overview
This module implements algorithms to compute FIRST and FOLLOW sets from any given context-free grammar, and constructs an LL(1) predictive parsing table.

## Target Grammar (from specification)
```
E   → T E'
E'  → + T E' | - T E' | ε
T   → F T'
T'  → * F T' | / F T' | ε
F   → ( E ) | id | num
```

## Expected Output

### FIRST Sets
```
FIRST(E)  = { (, id, num }
FIRST(E') = { +, -, ε }
FIRST(T)  = { (, id, num }
FIRST(T') = { *, /, ε }
FIRST(F)  = { (, id, num }
```

### FOLLOW Sets
```
FOLLOW(E)  = { $, ) }
FOLLOW(E') = { $, ) }
FOLLOW(T)  = { +, -, $, ) }
FOLLOW(T') = { +, -, $, ) }
FOLLOW(F)  = { *, /, +, -, $, ) }
```

## Programs Implemented

### 1. first_sets
Computes FIRST sets using the iterative algorithm:
- Initializes FIRST sets to empty
- For each production, applies FIRST computation rules
- Repeats until no changes occur (fixed point)
- Handles epsilon productions correctly

Usage:
```
./module4/first_sets <grammar_file>
```

### 2. follow_sets
Computes FOLLOW sets using the iterative algorithm:
- Initializes FOLLOW(start) with $
- For each production, applies FOLLOW rules
- Handles nullable symbols correctly
- Repeats until convergence

Usage:
```
./module4/follow_sets <grammar_file>
```

### 3. ll1_table
Builds the LL(1) predictive parsing table:
- Checks for LL(1) conflicts
- Constructs M[A, a] entries
- Exports table to CSV format
- Detects grammar incompatibilities

Usage:
```
./module4/ll1_table <grammar_file> [output.csv]
```

## Grammar File Format

Grammar files follow a simple text format:
```
# Comments start with #
# Each production on a single line: LHS -> RHS1 RHS2 ... | RHS_ALT1 RHS_ALT2 ...

E -> T E'
E' -> + T E' | - T E' | epsilon
T -> F T'
T' -> * F T' | / F T' | epsilon
F -> ( E ) | id | num
```

## LL(1) Compatibility Requirements
1. **No left recursion** - Grammar must be left-recursive free
2. **No ambiguity** - Grammar must be unambiguous
3. **No conflicts** - FIRST/FOLLOW sets must not create conflicts:
   - For A → α | β: FIRST(α) ∩ FIRST(β) = ∅
   - If NULLABLE(α): FIRST(β) ∩ FOLLOW(A) = ∅

## Algorithm Details

### FIRST Computation
For each nonterminal A:
1. If A → a... (terminal first), add 'a' to FIRST(A)
2. If A → B..., add FIRST(B) - {ε} to FIRST(A)
3. If all of X₁...Xₙ are nullable, add ε to FIRST(A)
4. Repeat until fixed point

### FOLLOW Computation
1. FOLLOW(start) = {$}
2. For each production A → X₁X₂...Xₙ, for each nonterminal B = Xᵢ:
   - Add FIRST(Xᵢ₊₁...Xₙ) - {ε} to FOLLOW(B)
   - If Xᵢ₊₁...Xₙ is nullable, add FOLLOW(A) to FOLLOW(B)
3. Repeat until fixed point

## Source Files
- `grammar.h/c` - Grammar representation and parsing
- `first_follow.h/c` - FIRST/FOLLOW computation
- `ll1_table.h/c` - LL(1) table construction
- `first_sets.c` - Main program for FIRST sets
- `follow_sets.c` - Main program for FOLLOW sets
- `ll1_table_main.c` - Main program for LL(1) table
- `grammar.txt` - Sample input grammar
