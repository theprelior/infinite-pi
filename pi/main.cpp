#include "pi.hpp"

#include <iostream>

using BigInt = pi::mp::cpp_int;

int main() {
    // Test base case k=0: P=1, Q=1, T=13591409
    auto r0 = pi::binary_split<BigInt>(0, 1);
    std::cout << "binary_split(0,1):\n";
    std::cout << "  P = " << r0.P << "\n";
    std::cout << "  Q = " << r0.Q << "\n";
    std::cout << "  T = " << r0.T << "\n";

    // Test base case k=1: p(1) = -(1*1*5) = -5
    auto r1 = pi::binary_split<BigInt>(1, 2);
    std::cout << "\nbinary_split(1,2):\n";
    std::cout << "  P = " << r1.P << "\n";
    std::cout << "  Q = " << r1.Q << "\n";
    std::cout << "  T = " << r1.T << "\n";

    // Test merge: binary_split(0,2) should equal merging (0,1) and (1,2)
    auto merged = pi::binary_split<BigInt>(0, 2);
    BigInt expected_T = r0.T * r1.Q + r0.P * r1.T;
    std::cout << "\nbinary_split(0,2) merge check:\n";
    std::cout << "  T from merge formula = " << expected_T << "\n";
    std::cout << "  T from binary_split  = " << merged.T << "\n";
    std::cout << "  Match: " << (expected_T == merged.T ? "YES" : "NO") << "\n";

    return 0;
}
