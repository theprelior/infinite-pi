#include "pi.hpp"

#include <iostream>

using BigInt = pi::mp::cpp_int;

int main() {
    // 1 term gives ~14 correct digits
    std::cout << "1 term:   " << pi::compute_pi<BigInt>(1, 15) << "\n";

    // 10 terms gives ~140 digits
    std::cout << "10 terms: " << pi::compute_pi<BigInt>(10, 50) << "\n";

    // Compute to 100 decimal places
    std::cout << "\n100 digits:\n" << pi::compute_pi<BigInt>(10, 100) << "\n";

    return 0;
}
