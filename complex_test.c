/* Complex Test: Functions, Loops, and Math */

int factorial(int n) {
    int result = 1;
    int i = 1;

    while (i <= n) {
        result = result * i;
        i = i + 1;
    }

    return result;
}

int fibonacci(int n) {
    int a = 0;
    int b = 1;
    int temp = 0;
    int i = 0;

    if (n == 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }

    while (i < n - 1) {
        temp = a + b;
        a = b;
        b = temp;
        i = i + 1;
    }

    return b;
}

int main() {
    int x = 5;
    int y = 10;
    int sum = 0;
    int fact = 0;
    int fib = 0;
    int i = 0;

    sum = x + y;
    fact = factorial(x);
    fib = fibonacci(y);

    if (sum > 10) {
        sum = sum * 2;
    } else {
        sum = sum + fact;
    }

    while (i < 5) {
        sum = sum + i;
        i = i + 1;
    }

    return sum + fib;
}
