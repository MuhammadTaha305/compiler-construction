int add(int x, int y) {
    return x + y;
}

int main() {
    int a;
    int b;
    int result;
    
    a = 10;
    b = 20;
    result = add(a, b);
    
    if (result > 25) {
        int sum = result + 5;
        return sum;
    }
    
    return result;
}
