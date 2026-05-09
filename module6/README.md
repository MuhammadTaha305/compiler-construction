# MODULE 6: INTERMEDIATE REPRESENTATION (THREE-ADDRESS CODE GENERATION)

## Overview
This module generates Three-Address Code (TAC) from the Abstract Syntax Tree produced by semantic analysis. TAC is a simple, low-level intermediate representation where each instruction has at most three operands.

## Three-Address Code Forms

### 1. Binary Operations
```
x = y op z          (addition, subtraction, multiplication, division, etc.)
```

### 2. Unary Operations
```
x = op y            (negation, logical not)
```

### 3. Copy Statements
```
x = y
```

### 4. Unconditional Jump
```
goto L
```

### 5. Conditional Jump
```
if x relop y goto L (relop: <, <=, >, >=, ==, !=)
```

### 6. Function Calls
```
param x             (pass parameter x)
call f, n           (call function f with n parameters)
return x            (return value x)
x = call f, n       (return value assignment)
```

### 7. Array Operations
```
x = a[i]            (array element access)
a[i] = x            (array element assignment)
```

### 8. Labels
```
L:                  (label definition)
function f:         (function definition)
```

## TAC Instruction Structure

Each instruction contains:
- **OpType**: Operation type (ADD, SUB, MUL, DIV, ASSIGN, etc.)
- **result**: Destination variable
- **operand1**: First source operand
- **operand2**: Second source operand
- **label**: Jump target (for goto/ifgoto)

## Temporary Variable Management

The TAC generator automatically creates temporary variables:
- Format: `t0`, `t1`, `t2`, ...
- Used for intermediate computations
- Eliminated by later optimization phases

Labels are generated as:
- Format: `L0`, `L1`, `L2`, ...
- Used for jump targets and loop control

## Source Files

### 1. tac.h
Header file defining:
- TAC instruction structure
- Temporary variable types
- Function prototypes

### 2. tac.c
Implementation of:
- Instruction emission functions
- Temporary variable generation
- TAC code output and printing
- CSV export functionality

### 3. tac_test.c
Comprehensive test demonstrating:
- Binary operations
- Conditional jumps and labels
- Function calls with parameters
- Array operations
- Loop generation with labels

## Programs

### tac_test
Generates TAC for two example programs:
1. **Simple arithmetic with conditional**
   - Variable assignment
   - Binary operation
   - Conditional jump
   - Function call

2. **Array processing loop**
   - Array indexing
   - Loop label generation
   - Temporary variables for intermediate values
   - Loop control flow

## Usage Example

```c
// Create TAC code generator
TACCode *tac = create_tac_code();

// Generate instructions
emit_instruction(tac, OP_ASSIGN, "a", "5", NULL);
emit_instruction(tac, OP_ASSIGN, "b", "3", NULL);
emit_instruction(tac, OP_ADD, "c", "a", "b");

// Generate conditional and labels
char *label_true = new_label(tac);
char *label_false = new_label(tac);
emit_ifgoto(tac, "c", ">", "7", label_true);
emit_goto(tac, label_false);

// Emit label and instructions
emit_label(tac, label_true);
emit_param(tac, "c");
emit_call(tac, "t0", "print", 1);

emit_label(tac, label_false);
emit_return(tac, "c");

// Print and export
print_tac_code_detailed(tac);
export_tac_code(tac, "output.txt");

// Cleanup
free_tac_code(tac);
```

## TAC Example Output

### Example 1: Simple Arithmetic with Conditional
```
function main:
   0: a = 5
   1: b = 3
   2: c = a + b
   3: if c > 7 goto L0
   4: goto L1
   5: L0:
   6: param c
   7: t0 = call print, 1
   8: L1:
   9: return c
```

### Example 2: Array Processing Loop
```
function array_demo:
   0: sum = 0
   1: i = 0
   2: L0:
   3: if i >= 10 goto L1
   4: t0 = i * 2
   5: arr[i] = t0
   6: t1 = arr[i]
   7: t2 = sum + t1
   8: sum = t2
   9: t3 = i + 1
  10: i = t3
  11: goto L0
  12: L1:
  13: return sum
```

## Operator Types

| OpType | Meaning | Format |
|--------|---------|--------|
| OP_ADD | Addition | x = y + z |
| OP_SUB | Subtraction | x = y - z |
| OP_MUL | Multiplication | x = y * z |
| OP_DIV | Division | x = y / z |
| OP_MOD | Modulo | x = y % z |
| OP_ASSIGN | Assignment | x = y |
| OP_UMINUS | Unary minus | x = -y |
| OP_LT | Less than | x = y < z |
| OP_LE | Less or equal | x = y <= z |
| OP_GT | Greater than | x = y > z |
| OP_GE | Greater or equal | x = y >= z |
| OP_EQ | Equal | x = y == z |
| OP_NE | Not equal | x = y != z |
| OP_AND | Logical AND | x = y && z |
| OP_OR | Logical OR | x = y \|\| z |
| OP_NOT | Logical NOT | x = !y |
| OP_ARRAY_REF | Array access | x = a[i] |
| OP_ARRAY_ASSIGN | Array assignment | a[i] = x |
| OP_GOTO | Unconditional jump | goto L |
| OP_IFGOTO | Conditional jump | if x op y goto L |
| OP_PARAM | Function parameter | param x |
| OP_CALL | Function call | x = call f, n |
| OP_RETURN | Return statement | return x |
| OP_FUNC | Function definition | function f: |
| OP_LABEL | Label definition | L: |

## Compilation

The TAC generator compiles directly without external dependencies:

```bash
gcc -o module6/tac_test module6/tac.c module6/tac_test.c -lm
```

## Output Formats

### 1. Console Output
Human-readable TAC listing with line numbers and operations in algebraic notation.

### 2. Detailed Output
Includes statistics:
- Total instructions
- Temporary variables used
- Labels generated

### 3. CSV Export
Machine-readable format for import into other tools.

## Deliverables

1. **Source files**: tac.h/c implementing TAC generation
2. **Test program**: tac_test.c demonstrating all TAC forms
3. **TAC listings**: 
   - tac_output.txt - Simple arithmetic example
   - tac_array_output.txt - Array processing example
4. **Documentation**: This README with usage examples

## Limitations and Future Extensions

Current limitations:
- Simple operand naming (no register allocation)
- No optimization passes
- Limited to 10,000 instructions
- No live variable analysis

Future extensions:
- Basic block generation
- Dominator tree construction
- Register allocation
- Peephole optimization
- Data flow analysis
