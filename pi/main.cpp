#include "pi.hpp"

#include <iostream>

using BigInt = pi::mp::cpp_int;

int main() {
    // Test that BigInt satisfies our Arithmetic concept
    static_assert(pi::Arithmetic<BigInt>, "BigInt must satisfy Arithmetic concept");

    // int and double also satisfy Arithmetic — concept is generic
    static_assert(pi::Arithmetic<int>, "int must satisfy Arithmetic concept");
    static_assert(pi::Arithmetic<double>, "double must satisfy Arithmetic concept");

    // Test SplitResult construction with BigInt
    pi::SplitResult<BigInt> r{BigInt(1), BigInt(1), BigInt(13591409)};
    std::cout << "SplitResult fields:\n";
    std::cout << "  P = " << r.P << "\n";
    std::cout << "  Q = " << r.Q << "\n";
    std::cout << "  T = " << r.T << "\n";

    // Test that BigInt handles large numbers correctly
    BigInt a("123456789012345678901234567890");
    BigInt b("987654321098765432109876543210");
    std::cout << "\nLarge number arithmetic:\n";
    std::cout << "  a     = " << a << "\n";
    std::cout << "  b     = " << b << "\n";
    std::cout << "  a + b = " << a + b << "\n";
    std::cout << "  a * b = " << a * b << "\n";

    // Test SplitResult with merge formula: T(a,b) = T(a,m)*Q(m,b) + P(a,m)*T(m,b)
    pi::SplitResult<BigInt> left {BigInt(2), BigInt(3), BigInt(5)};
    pi::SplitResult<BigInt> right{BigInt(7), BigInt(11), BigInt(13)};
    BigInt merged_T = left.T * right.Q + left.P * right.T;
    std::cout << "\nMerge formula test:\n";
    std::cout << "  left.T * right.Q + left.P * right.T = " << merged_T << "\n";

    return 0;
}
