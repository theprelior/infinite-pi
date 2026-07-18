#include "pi.hpp"

#include <cassert>
#include <iostream>
#include <string>

using BigInt = pi::mp::cpp_int;

static void test_concepts() {
    static_assert(pi::Arithmetic<BigInt>, "BigInt must satisfy Arithmetic");
    static_assert(pi::Arithmetic<int>,    "int must satisfy Arithmetic");
    static_assert(pi::Arithmetic<double>, "double must satisfy Arithmetic");
    std::cout << "[PASS] concepts\n";
}

static void test_binary_split_base_cases() {
    auto r0 = pi::binary_split<BigInt>(0, 1);
    assert(r0.P == 1);
    assert(r0.Q == 1);
    assert(r0.T == 13591409);

    auto r1 = pi::binary_split<BigInt>(1, 2);
    assert(r1.P == -5);

    std::cout << "[PASS] binary_split base cases\n";
}

static void test_binary_split_merge() {
    auto left  = pi::binary_split<BigInt>(0, 1);
    auto right = pi::binary_split<BigInt>(1, 2);
    BigInt expected_T = left.T * right.Q + left.P * right.T;

    auto merged = pi::binary_split<BigInt>(0, 2);
    assert(merged.T == expected_T);

    std::cout << "[PASS] binary_split merge\n";
}

static void test_isqrt() {
    assert(pi::isqrt<BigInt>(BigInt(0))   == 0);
    assert(pi::isqrt<BigInt>(BigInt(1))   == 1);
    assert(pi::isqrt<BigInt>(BigInt(9))   == 3);
    assert(pi::isqrt<BigInt>(BigInt(10))  == 3);
    assert(pi::isqrt<BigInt>(BigInt(100)) == 10);

    BigInt big = pi::mp::pow(BigInt(10), 52);
    assert(pi::isqrt<BigInt>(big) == pi::mp::pow(BigInt(10), 26));

    std::cout << "[PASS] isqrt\n";
}

static void test_estimate_terms() {
    assert(pi::estimate_terms(50)   ==  6);
    assert(pi::estimate_terms(100)  == 10);
    assert(pi::estimate_terms(1000) == 73);
    std::cout << "[PASS] estimate_terms\n";
}

static void test_compute_pi_prefix() {
    std::string pi50 = pi::compute_pi<BigInt>(pi::estimate_terms(50), 50);
    assert(pi50.substr(0, 7) == "3.14159");

    std::string pi100 = pi::compute_pi<BigInt>(pi::estimate_terms(100), 100);
    assert(pi100.size() == 102); // "3." + 100 digits
    assert(pi100.substr(0, 7) == "3.14159");

    std::cout << "[PASS] compute_pi prefix\n";
}

int main() {
    test_concepts();
    test_binary_split_base_cases();
    test_binary_split_merge();
    test_isqrt();
    test_estimate_terms();
    test_compute_pi_prefix();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
