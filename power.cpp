#include <iostream>

// Helper function to compute the square of a number
long long square(long long x) {
    return x * x;
}

// Recursive function for fast exponentiation
long long fast_expt(long long b, long long n) {
    long long res = (n == 0) ? 1 : 
                          (n % 2 == 0) 
                            ? square(fast_expt(b, n / 2)) 
                            : b * fast_expt(b, n - 1);
    return res;
}

int main() {
    long long base = 2;
    long long exp = 40;

    long long result = fast_expt(base, exp);

    std::cout << base << "^" << exp << " = " << result << std::endl;

    return 0;
}
