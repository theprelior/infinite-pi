#include "pi.hpp"

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#ifdef PI_USE_GMP_BACKEND
#include <boost/multiprecision/gmp.hpp>
using BigInt = pi::mp::mpz_int;
#else
using BigInt = pi::mp::cpp_int;
#endif

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

// Run gmp-chudnovsky subprocess and return wall-clock time in ms.
// Returns -1.0 if the binary is not found.
static double time_gmp(int digits, const std::string& gmp_path) {
    std::string cmd = gmp_path + " " + std::to_string(digits) + " 0 >/dev/null 2>&1";
    pi::Timer t;
    int ret = std::system(cmd.c_str());
    double ms = t.elapsed_ms();
    return (ret == 0) ? ms : -1.0;
}

static void print_separator(int width) {
    std::cout << std::string(width, '-') << "\n";
}

int main(int argc, char** argv) {
    // Optional: pass path to gmp-chudnovsky as first argument
    std::string gmp_path = (argc >= 2)
        ? argv[1]
        : "./build/pi/gmp-chudnovsky";

    bool gmp_available = (std::system((gmp_path + " 100 0 >/dev/null 2>&1").c_str()) == 0);

    std::vector<int> sizes = {100, 500, 1000, 5000, 10000};

    // ── Section 1: Sequential vs Parallel ───────────────────────────────────
#ifdef PI_USE_GMP_BACKEND
    std::cout << "\n=== Sequential vs Parallel (Boost mpz_int / GMP) ===\n\n";
#else
    std::cout << "\n=== Sequential vs Parallel (Boost cpp_int) ===\n\n";
#endif
    std::cout << std::left
              << std::setw(10) << "Digits"
              << std::setw(16) << "Sequential(ms)"
              << std::setw(16) << "Parallel(ms)"
              << "Speedup\n";
    print_separator(52);

    for (int d : sizes) {
        auto r      = run_once(d);
        double sp   = r.seq_ms / r.par_ms;
        std::cout << std::left
                  << std::setw(10) << r.digits
                  << std::setw(16) << std::fixed << std::setprecision(2) << r.seq_ms
                  << std::setw(16) << r.par_ms
                  << std::setprecision(2) << sp << "x\n";
    }

    // ── Section 2: Boost vs GMP ──────────────────────────────────────────────
    std::cout << "\n=== Boost cpp_int vs GMP (gmp-chudnovsky reference) ===\n\n";

    if (!gmp_available) {
        std::cout << "gmp-chudnovsky not found at: " << gmp_path << "\n"
                  << "Build with: cmake -DBUILD_REFERENCE=ON -S . -B build\n";
        return 0;
    }

    std::cout << std::left
              << std::setw(10) << "Digits"
              << std::setw(16) << "Boost(ms)"
              << std::setw(16) << "GMP(ms)"
              << "GMP speedup\n";
    print_separator(52);

    for (int d : sizes) {
        int terms      = pi::estimate_terms(d);
        pi::Timer tb;
        pi::compute_pi<BigInt>(terms, d);
        double boost_ms = tb.elapsed_ms();

        double gmp_ms = time_gmp(d, gmp_path);

        double ratio   = boost_ms / gmp_ms;
        std::string winner = (ratio > 1.0) ? "GMP faster" : "Boost faster";
        std::cout << std::left
                  << std::setw(10) << d
                  << std::setw(16) << std::fixed << std::setprecision(2) << boost_ms
                  << std::setw(16) << gmp_ms
                  << std::setprecision(1)
                  << (ratio > 1.0 ? ratio : 1.0 / ratio)
                  << "x  " << winner << "\n";
    }

    std::cout << "\nNote: GMP uses hand-optimized assembly (Karatsuba, FFT multiplication).\n"
              << "      Boost cpp_int uses schoolbook O(n^2) multiplication.\n"
              << "      The gap grows with digit count as multiplication dominates.\n";

    return 0;
}
