#include "pi.hpp"

#include <cstring>
#include <expected>
#include <format>
#include <iostream>
#include <stdexcept>

using BigInt = pi::mp::cpp_int;

/// @brief Holds parsed command-line arguments.
struct Args {
    int  num_digits   = 100;
    bool use_parallel = false;
    bool use_sqrt     = false;
};

/// @brief Parses command-line arguments into an Args struct.
///
/// Returns std::unexpected with a human-readable error string on invalid input,
/// avoiding exception-based control flow at the call site.
///
/// @param argc Argument count from main.
/// @param argv Argument vector from main.
/// @return Parsed Args on success, error description on failure.
std::expected<Args, std::string> parse_args(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--parallel") == 0) {
            args.use_parallel = true;
        } else if (std::strcmp(argv[i], "--sqrt") == 0) {
            args.use_sqrt = true;
        } else {
            try {
                args.num_digits = std::stoi(argv[i]);
                if (args.num_digits <= 0) { throw std::out_of_range("must be positive"); }
            } catch (const std::invalid_argument&) {
                return std::unexpected(std::format("'{}' is not a valid number", argv[i]));
            } catch (const std::out_of_range&) {
                return std::unexpected(std::string("digit count must be a positive integer"));
            }
        }
    }
    return args;
}

int main(int argc, char** argv) {
    auto result = parse_args(argc, argv);
    if (!result) {
        std::cerr << std::format("Error: {}\n", result.error());
        return 1;
    }

    const Args& args = *result;
    const int num_terms = pi::estimate_terms(args.num_digits);

    std::cout << std::format("Digits requested : {}\n", args.num_digits);
    std::cout << std::format("Terms needed     : {}\n", num_terms);
    std::string mode;
    if (args.use_sqrt)          { mode = "sqrt(pi)";   }
    else if (args.use_parallel) { mode = "parallel";   }
    else                        { mode = "sequential"; }
    std::cout << std::format("Mode             : {}\n", mode);

    try {
        pi::Timer t;

        if (args.use_parallel) {
            auto split = pi::parallel_binary_split<BigInt>(0, num_terms);
            std::cout << std::format("pi = {}\n",
                pi::compute_pi_from_split<BigInt>(split, args.num_digits));
        } else {
            pi::PiComputer<BigInt> computer(args.num_digits);
            if (args.use_sqrt) {
                std::cout << std::format("sqrt(pi) = {}\n", computer.sqrt_pi());
            } else {
                std::cout << std::format("pi = {}\n", computer.pi());
            }
        }

        std::cout << std::format("Time             : {:.2f} ms\n", t.elapsed_ms());

    } catch (const std::bad_alloc&) {
        std::cerr << std::format("Error: not enough memory for {} digits\n", args.num_digits);
        return 1;
    }

    return 0;
}
