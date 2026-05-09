#include "tac.h"

int main() {
    TACCode *tac = create_tac_code();

    printf("\n--- Generating TAC for: ---\n");
    printf("int main() {\n");
    printf("    int a, b, c;\n");
    printf("    a = 5;\n");
    printf("    b = 3;\n");
    printf("    c = a + b;\n");
    printf("    if (c > 7) {\n");
    printf("        print(c);\n");
    printf("    }\n");
    printf("    return c;\n");
    printf("}\n\n");

    // Generate function prologue
    emit_function(tac, "main");

    // Variable assignments
    emit_instruction(tac, OP_ASSIGN, "a", "5", NULL);
    emit_instruction(tac, OP_ASSIGN, "b", "3", NULL);

    // Binary operation: c = a + b
    emit_instruction(tac, OP_ADD, "c", "a", "b");

    // Comparison: if c > 7
    char *label_true = new_label(tac);
    char *label_false = new_label(tac);
    emit_ifgoto(tac, "c", ">", "7", label_true);
    emit_goto(tac, label_false);

    // True branch
    emit_label(tac, label_true);
    emit_param(tac, "c");
    emit_call(tac, "t0", "print", 1);

    // End of if
    emit_label(tac, label_false);

    // Return statement
    emit_return(tac, "c");

    // Print the generated code
    print_tac_code_detailed(tac);

    // Export to file
    export_tac_code(tac, "module6/tac_output.txt");
    printf("\n\nTAC code exported to: module6/tac_output.txt\n");

    // Cleanup
    free_tac_code(tac);

    return 0;
}
