#include "pi.hpp"

#include <iostream>

using BigInt = pi::mp::cpp_int;

int main() {
    // Basic isqrt tests
    std::cout << "isqrt tests:\n";
    std::cout << "  isqrt(0)     = " << pi::isqrt<BigInt>(BigInt(0))   << "  (expected 0)\n";
    std::cout << "  isqrt(1)     = " << pi::isqrt<BigInt>(BigInt(1))   << "  (expected 1)\n";
    std::cout << "  isqrt(9)     = " << pi::isqrt<BigInt>(BigInt(9))   << "  (expected 3)\n";
    std::cout << "  isqrt(10)    = " << pi::isqrt<BigInt>(BigInt(10))  << "  (expected 3, floor)\n";
    std::cout << "  isqrt(100)   = " << pi::isqrt<BigInt>(BigInt(100)) << "  (expected 10)\n";

    // Test with a large number
    BigInt large("10000000000000000000000000000000000000000000000000000");
    std::cout << "\n  isqrt(10^52) = " << pi::isqrt<BigInt>(large) << "\n";
    std::cout << "  (expected 100000000000000000000000000)\n";

    // Test the value we actually need: isqrt(10005)
    std::cout << "\n  isqrt(10005) = " << pi::isqrt<BigInt>(BigInt(10005)) << "\n";
    std::cout << "  (100^2=10000, 101^2=10201 → floor is 100)\n";

    return 0;
}
