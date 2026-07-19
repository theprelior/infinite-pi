#include "pi.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

using BigInt = pi::mp::cpp_int;

struct BenchResult {
    int    digits;
    double seq_ms;
    double par_ms;
};

static BenchResult run_once(int digits) {
    int terms = pi::estimate_terms(digits);

    pi::Timer t1;
    pi::compute_pi<BigInt>(terms, digits);
    double seq = t1.elapsed_ms();

    pi::Timer t2;
    auto split = pi::parallel_binary_split<BigInt>(0, terms);
    pi::compute_pi_from_split<BigInt>(split, digits);
    double par = t2.elapsed_ms();

    return {digits, seq, par};
}

int main() {
    std::vector<int> sizes = {100, 500, 1000, 5000, 10000};

    std::cout << std::left
              << std::setw(10) << "Digits"
              << std::setw(14) << "Sequential"
              << std::setw(14) << "Parallel"
              << "Speedup\n";
    std::cout << std::string(52, '-') << "\n";

    for (int d : sizes) {
        auto r = run_once(d);
        double speedup = r.seq_ms / r.par_ms;
        std::cout << std::left
                  << std::setw(10) << r.digits
                  << std::setw(14) << std::fixed << std::setprecision(2) << r.seq_ms
                  << std::setw(14) << r.par_ms
                  << std::setprecision(2) << speedup << "x\n";
    }

    return 0;
}
