#include "pi.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>

using BigInt = pi::mp::cpp_int;

int main(int argc, char** argv) {
    int  num_digits   = 100;
    bool use_parallel = false;
    bool use_sqrt     = false;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--parallel") == 0) {
            use_parallel = true;
        } else if (std::strcmp(argv[i], "--sqrt") == 0) {
            use_sqrt = true;
        } else {
            try {
                num_digits = std::stoi(argv[i]);
                if (num_digits <= 0) {
                    throw std::out_of_range("must be positive");
                }
            } catch (const std::invalid_argument&) {
                std::cerr << "Error: '" << argv[i] << "' is not a valid number\n";
                return 1;
            } catch (const std::out_of_range&) {
                std::cerr << "Error: digit count must be a positive integer\n";
                return 1;
            }
        }
    }

    int num_terms = pi::estimate_terms(num_digits);

    std::cout << "Digits requested : " << num_digits << "\n";
    std::cout << "Terms needed     : " << num_terms  << "\n";
    std::cout << "Mode             : "
              << (use_sqrt ? "sqrt(pi)" : (use_parallel ? "parallel" : "sequential")) << "\n";

    try {
        pi::Timer t;

        if (use_parallel) {
            auto split = pi::parallel_binary_split<BigInt>(0, num_terms);
            std::cout << "pi = " << pi::compute_pi_from_split<BigInt>(split, num_digits) << "\n";
        } else {
            pi::PiComputer<BigInt> computer(num_digits);
            if (use_sqrt) {
                std::cout << "sqrt(pi) = " << computer.sqrt_pi() << "\n";
            } else {
                std::cout << "pi = " << computer.pi() << "\n";
            }
        }

        std::cout << "Time             : " << t.elapsed_ms() << " ms\n";

    } catch (const std::bad_alloc&) {
        std::cerr << "Error: not enough memory for " << num_digits << " digits\n";
        return 1;
    }

    return 0;
}
