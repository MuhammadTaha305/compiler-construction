#include "first_follow.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <grammar_file>\n", argv[0]);
        fprintf(stderr, "Example: %s grammar.txt\n", argv[0]);
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

    free_first_sets(fs);
    free_follow_sets(fos);
    free_grammar(g);

    return 0;
}
