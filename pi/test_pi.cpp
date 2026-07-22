#include "pi.hpp"

#include <catch2/catch_test_macros.hpp>

using BigInt = pi::mp::cpp_int;

TEST_CASE("Arithmetic concept is satisfied", "[concepts]") {
    STATIC_REQUIRE(pi::Arithmetic<BigInt>);
    STATIC_REQUIRE(pi::Arithmetic<int>);
    STATIC_REQUIRE(pi::Arithmetic<double>);
}

TEST_CASE("binary_split base cases", "[binary_split]") {
    SECTION("k=0: P=1, Q=1, T=13591409") {
        auto r = pi::binary_split<BigInt>(0, 1);
        REQUIRE(r.P == 1);
        REQUIRE(r.Q == 1);
        REQUIRE(r.T == 13591409);
    }

    SECTION("k=1: P=-5") {
        auto r = pi::binary_split<BigInt>(1, 2);
        REQUIRE(r.P == -5);
    }
}

TEST_CASE("binary_split merge formula", "[binary_split]") {
    auto left  = pi::binary_split<BigInt>(0, 1);
    auto right = pi::binary_split<BigInt>(1, 2);
    BigInt expected_T = left.T * right.Q + left.P * right.T;

    auto merged = pi::binary_split<BigInt>(0, 2);
    REQUIRE(merged.T == expected_T);
}

TEST_CASE("isqrt correctness", "[isqrt]") {
    REQUIRE(pi::isqrt<BigInt>(BigInt(0))   == 0);
    REQUIRE(pi::isqrt<BigInt>(BigInt(1))   == 1);
    REQUIRE(pi::isqrt<BigInt>(BigInt(9))   == 3);
    REQUIRE(pi::isqrt<BigInt>(BigInt(10))  == 3);
    REQUIRE(pi::isqrt<BigInt>(BigInt(100)) == 10);

    BigInt big = pi::mp::pow(BigInt(10), 52);
    REQUIRE(pi::isqrt<BigInt>(big) == pi::mp::pow(BigInt(10), 26));
}

TEST_CASE("estimate_terms", "[estimate_terms]") {
    REQUIRE(pi::estimate_terms(50)   ==  6);
    REQUIRE(pi::estimate_terms(100)  == 10);
    REQUIRE(pi::estimate_terms(1000) == 73);
}

TEST_CASE("compute_pi output", "[compute_pi]") {
    SECTION("50 digits start with 3.14159") {
        auto pi50 = pi::compute_pi<BigInt>(pi::estimate_terms(50), 50);
        REQUIRE(pi50.substr(0, 7) == "3.14159");
    }

    SECTION("100 digits correct length") {
        auto pi100 = pi::compute_pi<BigInt>(pi::estimate_terms(100), 100);
        REQUIRE(pi100.size() == 102);
        REQUIRE(pi100.substr(0, 7) == "3.14159");
    }
}

TEST_CASE("PiComputer", "[PiComputer]") {
    pi::PiComputer<BigInt> computer(50);

    SECTION("pi() starts with 3.14159") {
        REQUIRE(computer.pi().substr(0, 7) == "3.14159");
    }

    SECTION("sqrt_pi() starts with 1.77245385") {
        REQUIRE(computer.sqrt_pi().substr(0, 10) == "1.77245385");
    }
}

TEST_CASE("compute_sqrt_pi", "[sqrt_pi]") {
    auto sqpi = pi::compute_sqrt_pi<BigInt>(pi::estimate_terms(30), 30);
    REQUIRE(sqpi.substr(0, 10) == "1.77245385");
    REQUIRE(static_cast<int>(sqpi.size()) == 32);
}
