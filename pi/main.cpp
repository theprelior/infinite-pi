#include "pi.hpp"

#include <iostream>

using BigInt = pi::mp::cpp_int;

int main() {
    // Test that BigInt satisfies our Arithmetic concept
    static_assert(pi::Arithmetic<BigInt>, "BigInt must satisfy Arithmetic concept");

    // Test that SplitResult can be constructed with BigInt
    pi::SplitResult<BigInt> r{BigInt(1), BigInt(1), BigInt(13591409)};

    std::cout << "P = " << r.P << "\n";
    std::cout << "Q = " << r.Q << "\n";
    std::cout << "T = " << r.T << "\n";

    return 0;
}
