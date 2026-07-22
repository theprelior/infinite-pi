#include "pi.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string_view>

#ifdef PI_USE_GMP_BACKEND
#include <boost/multiprecision/gmp.hpp>
using BigInt = pi::mp::mpz_int;
#else
using BigInt = pi::mp::cpp_int;
#endif

// First 100 verified decimal digits of pi (NIST reference).
static constexpr std::string_view PI_KNOWN_100 =
    "3.14159265358979323846264338327950288419716939937510"
    "58209749445923078164062862089986280348253421170679";

// First 50 verified decimal digits of sqrt(pi).
static constexpr std::string_view SQRT_PI_KNOWN_50 =
    "1.77245385090551602729816748334114518279754945612238";

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

TEST_CASE("binary_split P is multiplicative", "[binary_split]") {
    auto r02 = pi::binary_split<BigInt>(0, 2);
    auto r01 = pi::binary_split<BigInt>(0, 1);
    auto r12 = pi::binary_split<BigInt>(1, 2);
    REQUIRE(r02.P == r01.P * r12.P);
    REQUIRE(r02.Q == r01.Q * r12.Q);
}

TEST_CASE("isqrt correctness", "[isqrt]") {
    REQUIRE(pi::isqrt<BigInt>(BigInt(0))   == 0);
    REQUIRE(pi::isqrt<BigInt>(BigInt(1))   == 1);
    REQUIRE(pi::isqrt<BigInt>(BigInt(4))   == 2);
    REQUIRE(pi::isqrt<BigInt>(BigInt(9))   == 3);
    REQUIRE(pi::isqrt<BigInt>(BigInt(10))  == 3);
    REQUIRE(pi::isqrt<BigInt>(BigInt(25))  == 5);
    REQUIRE(pi::isqrt<BigInt>(BigInt(144)) == 12);
    REQUIRE(pi::isqrt<BigInt>(BigInt(100)) == 10);

    BigInt big = pi::mp::pow(BigInt(10), 52);
    REQUIRE(pi::isqrt<BigInt>(big) == pi::mp::pow(BigInt(10), 26));
}

TEST_CASE("isqrt floor property", "[isqrt]") {
    for (int n : {2, 3, 5, 6, 7, 8, 15, 24, 35}) {
        BigInt s = pi::isqrt<BigInt>(BigInt(n));
        REQUIRE(s * s <= n);
        REQUIRE((s + 1) * (s + 1) > n);
    }
}

TEST_CASE("estimate_terms", "[estimate_terms]") {
    REQUIRE(pi::estimate_terms(50)   ==  6);
    REQUIRE(pi::estimate_terms(100)  == 10);
    REQUIRE(pi::estimate_terms(1000) == 73);
}

TEST_CASE("estimate_terms grows with digits", "[estimate_terms]") {
    REQUIRE(pi::estimate_terms(100) < pi::estimate_terms(200));
    REQUIRE(pi::estimate_terms(200) < pi::estimate_terms(1000));
}

TEST_CASE("estimate_terms is constexpr", "[constexpr]") {
    constexpr int terms = pi::estimate_terms(100);
    STATIC_REQUIRE(terms > 0);
    STATIC_REQUIRE(pi::estimate_terms(1000) > pi::estimate_terms(100));
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

    SECTION("200 digits correct prefix") {
        auto pi200 = pi::compute_pi<BigInt>(pi::estimate_terms(200), 200);
        REQUIRE(pi200.size() == 202);
        REQUIRE(pi200.substr(0, 16) == "3.14159265358979");
    }
}

TEST_CASE("pi matches verified reference digits", "[verification]") {
    auto computed = pi::compute_pi<BigInt>(pi::estimate_terms(100), 100);
    REQUIRE(computed == PI_KNOWN_100);
}

TEST_CASE("sqrt(pi) matches verified reference digits", "[verification]") {
    auto computed = pi::compute_sqrt_pi<BigInt>(pi::estimate_terms(50), 50);
    REQUIRE(computed == SQRT_PI_KNOWN_50);
}

TEST_CASE("compute_pi_from_split matches compute_pi", "[compute_pi]") {
    int digits = 50;
    int terms  = pi::estimate_terms(digits);
    auto split = pi::binary_split<BigInt>(0, terms);

    auto via_split   = pi::compute_pi_from_split<BigInt>(split, digits);
    auto via_compute = pi::compute_pi<BigInt>(terms, digits);

    REQUIRE(via_split == via_compute);
}

TEST_CASE("compute_pi_checked — std::expected", "[expected]") {
    SECTION("valid input returns value") {
        auto r = pi::compute_pi_checked<BigInt>(pi::estimate_terms(50), 50);
        REQUIRE(r.has_value());
        REQUIRE(r->substr(0, 7) == "3.14159");
    }

    SECTION("zero digits returns unexpected") {
        auto r = pi::compute_pi_checked<BigInt>(5, 0);
        REQUIRE_FALSE(r.has_value());
        REQUIRE_FALSE(r.error().empty());
    }

    SECTION("negative digits returns unexpected") {
        auto r = pi::compute_pi_checked<BigInt>(5, -1);
        REQUIRE_FALSE(r.has_value());
    }
}

TEST_CASE("compute_sqrt_pi_checked — std::expected", "[expected]") {
    SECTION("valid input returns value") {
        auto r = pi::compute_sqrt_pi_checked<BigInt>(pi::estimate_terms(30), 30);
        REQUIRE(r.has_value());
        REQUIRE(r->substr(0, 10) == "1.77245385");
    }

    SECTION("zero digits returns unexpected") {
        auto r = pi::compute_sqrt_pi_checked<BigInt>(5, 0);
        REQUIRE_FALSE(r.has_value());
    }
}

TEST_CASE("parallel_binary_split matches sequential", "[parallel]") {
    int digits = 100;
    int terms  = pi::estimate_terms(digits);

    auto seq = pi::binary_split<BigInt>(0, terms);
    auto par = pi::parallel_binary_split<BigInt>(0, terms);

    REQUIRE(seq.P == par.P);
    REQUIRE(seq.Q == par.Q);
    REQUIRE(seq.T == par.T);
}

TEST_CASE("parallel compute_pi matches sequential", "[parallel]") {
    int digits = 50;
    int terms  = pi::estimate_terms(digits);

    auto par_split = pi::parallel_binary_split<BigInt>(0, terms);
    auto pi_par    = pi::compute_pi_from_split<BigInt>(par_split, digits);
    auto pi_seq    = pi::compute_pi<BigInt>(terms, digits);

    REQUIRE(pi_par == pi_seq);
}

TEST_CASE("parallel correctness across digit sizes", "[parallel]") {
    for (int digits : {50, 100, 200}) {
        int terms      = pi::estimate_terms(digits);
        auto par_split = pi::parallel_binary_split<BigInt>(0, terms);
        auto seq_split = pi::binary_split<BigInt>(0, terms);
        REQUIRE(par_split.P == seq_split.P);
        REQUIRE(par_split.Q == seq_split.Q);
        REQUIRE(par_split.T == seq_split.T);
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

    SECTION("pi() result length") {
        REQUIRE(computer.pi().size() == 52);
    }
}

TEST_CASE("PiComputer pi and sqrt_pi independent calls agree", "[PiComputer]") {
    pi::PiComputer<BigInt> c(100);
    REQUIRE(c.pi().substr(0, 7) == "3.14159");
    REQUIRE(c.sqrt_pi().substr(0, 10) == "1.77245385");
    REQUIRE(c.pi() == c.pi());
}

TEST_CASE("compute_sqrt_pi", "[sqrt_pi]") {
    auto sqpi = pi::compute_sqrt_pi<BigInt>(pi::estimate_terms(30), 30);
    REQUIRE(sqpi.substr(0, 10) == "1.77245385");
    REQUIRE(static_cast<int>(sqpi.size()) == 32);
}

TEST_CASE("compute_sqrt_pi longer precision", "[sqrt_pi]") {
    auto sqpi = pi::compute_sqrt_pi<BigInt>(pi::estimate_terms(100), 100);
    REQUIRE(sqpi.substr(0, 16) == "1.77245385090551");
    REQUIRE(static_cast<int>(sqpi.size()) == 102);
}

TEST_CASE("Timer elapsed is positive", "[timer]") {
    pi::Timer t;
    double ms = t.elapsed_ms();
    REQUIRE(ms >= 0.0);
}

TEST_CASE("Timer increases over time", "[timer]") {
    pi::Timer t;
    pi::compute_pi<BigInt>(pi::estimate_terms(100), 100);
    REQUIRE(t.elapsed_ms() > 0.0);
}
