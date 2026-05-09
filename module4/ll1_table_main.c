#include "ll1_table.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <grammar_file> [output_csv]\n", argv[0]);
        fprintf(stderr, "Example: %s grammar.txt ll1_table.csv\n", argv[0]);
        return 1;
    }

    Grammar *g = parse_grammar_from_file(argv[1]);
    if (!g) {
        fprintf(stderr, "Failed to parse grammar\n");
        return 1;
    }

    print_grammar(g);

    FirstSets *fs = compute_first_sets(g);
    print_first_sets(fs, g);

    FollowSets *fos = compute_follow_sets(g, fs);
    print_follow_sets(fos, g);

    printf("\nCHECKING LL(1) COMPATIBILITY\n");
    int conflicts = check_ll1_conflicts(g, fs, fos);
    if (conflicts == 0) {
        printf("Grammar is LL(1) compatible (no conflicts)\n");
    } else {
        printf("Grammar has %d conflict(s) - NOT LL(1)\n", conflicts);
    }

    LL1ParseTable *table = build_ll1_table(g, fs, fos);
    print_ll1_table(table, g);

    if (argc > 2) {
        print_ll1_table_csv(table, g, argv[2]);
        printf("\nLL(1) table exported to: %s\n", argv[2]);
    }

    free_ll1_table(table);
    free_first_sets(fs);
    free_follow_sets(fos);
    free_grammar(g);

    return 0;
}
