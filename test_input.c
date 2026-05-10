/* Test Program: Optimization Opportunities Demo */

int main() {
    /* Constant Folding: 2 + 3 should become 5 at compile time */
    int a = 2 + 3;
    int b = 10 * 2;
    int c = 100 / 4;

    /* Constant Propagation: x=5, so y should become 5+10=15 */
    int x = 5;
    int y = x + 10;
    int z = x * 2;

    /* Common Subexpression: (a + b) computed twice */
    int p = a + b;
    int q = a + b;
    int r = p + q;

    /* Dead Code: unused variables */
    int dead1 = 100;
    int dead2 = 200;
    int dead3 = dead1 + dead2;

    /* More constant folding in expressions */
    int result = (2 * 3) + (4 * 5);

    /* Loop with invariant code */
    int sum = 0;
    int invariant = 10 * 5;
    int i = 0;

    while (i < 10) {
        sum = sum + invariant;
        i = i + 1;
    }

    /* Only result and sum are actually used */
    return result + sum;
}
