#include "pi.hpp"

#include <cstdlib>
#include <iostream>

using BigInt = pi::mp::cpp_int;

int main(int argc, char** argv) {
    // Default: 50 digits
    int num_digits = 50;
    if (argc == 2) {
        num_digits = std::atoi(argv[1]);
    }

    int num_terms = pi::estimate_terms(num_digits);

    std::cout << "Digits requested : " << num_digits << "\n";
    std::cout << "Terms needed     : " << num_terms  << "\n";
    std::cout << "pi = " << pi::compute_pi<BigInt>(num_terms, num_digits) << "\n";

    return 0;
}
