#include "pi.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>

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
            num_digits = std::atoi(argv[i]);
        }
    }

    int num_terms = pi::estimate_terms(num_digits);

    std::cout << "Digits requested : " << num_digits << "\n";
    std::cout << "Terms needed     : " << num_terms  << "\n";
    std::cout << "Mode             : "
              << (use_sqrt ? "sqrt(pi)" : (use_parallel ? "parallel" : "sequential")) << "\n";

    pi::Timer t;

    if (use_parallel) {
        auto split  = pi::parallel_binary_split<BigInt>(0, num_terms);
        std::string result = pi::compute_pi_from_split<BigInt>(split, num_digits);
        std::cout << "pi = " << result << "\n";
    } else {
        pi::PiComputer<BigInt> computer(num_digits);
        if (use_sqrt) {
            std::cout << "sqrt(pi) = " << computer.sqrt_pi() << "\n";
        } else {
            std::cout << "pi = " << computer.pi() << "\n";
        }
    }

    std::cout << "Time             : " << t.elapsed_ms() << " ms\n";

    return 0;
}
