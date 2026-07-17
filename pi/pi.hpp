#pragma once

#include <boost/multiprecision/cpp_int.hpp>

#include <chrono>
#include <cmath>
#include <concepts>
#include <string>

namespace pi {

namespace mp = boost::multiprecision;

template <typename T>
concept Multipliable = requires(T a, T b) {
    { a * b } -> std::convertible_to<T>;
};

template <typename T>
concept Arithmetic = Multipliable<T> && requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { a - b } -> std::convertible_to<T>;
};

// Holds the three values produced by binary splitting: P, Q, T
template <Arithmetic IntType>
struct SplitResult {
    IntType P;
    IntType Q;
    IntType T;
};

// Chudnovsky binary splitting
//
// 1/pi = (sqrt(10005) / 4270934400) * sum_{k=0}^{inf} T(k)
//
// Instead of computing each term separately, binary splitting
// accumulates P, Q, T recursively and does one division at the end.
//
// Base case (k=0): P=1, Q=1, T=13591409
// Base case (k>0): p(k) = -(6k-5)(2k-1)(6k-1)
//                  q(k) = k^3 * 10939058860032000
//                  t(k) = (13591409 + 545140134*k) * p(k)
//
// Merge:  P(a,b) = P(a,m) * P(m,b)
//         Q(a,b) = Q(a,m) * Q(m,b)
//         T(a,b) = T(a,m) * Q(m,b) + P(a,m) * T(m,b)

template <Arithmetic IntType>
SplitResult<IntType> binary_split(int a, int b) {
    if (b - a == 1) {
        if (a == 0) {
            return {IntType(1), IntType(1), IntType(13591409)};
        }
        IntType k(a);
        IntType p = -(IntType(6 * a - 5) * IntType(2 * a - 1) * IntType(6 * a - 1));
        IntType q = k * k * k * IntType("10939058860032000");
        IntType t = (IntType(13591409) + IntType(545140134) * k) * p;
        return {p, q, t};
    }

    int mid = (a + b) / 2;
    auto left  = binary_split<IntType>(a, mid);
    auto right = binary_split<IntType>(mid, b);

    return {
        left.P * right.P,
        left.Q * right.Q,
        left.T * right.Q + left.P * right.T
    };
}

// Integer square root using Newton's method: returns floor(sqrt(n))
// Normal sqrt() works on double — loses precision for large BigInt values.
// Newton iteration: x_{i+1} = (x_i + n / x_i) / 2  converges to floor(sqrt(n))

template <Arithmetic IntType>
IntType isqrt(const IntType& n) {
    if (n <= 0) {
        return IntType(0);
    }

    IntType x = IntType(1) << (mp::msb(n) / 2 + 1);
    while (true) {
        IntType next = (x + n / x) / 2;
        if (next >= x) {
            break;
        }
        x = next;
    }
    return x;
}

// Compute pi to num_digits decimal places using num_terms Chudnovsky terms.
//
// From binary splitting:  1/pi = sqrt(10005)/4270934400 * T/Q
// Rearranged:             pi   = 4270934400 * Q / (sqrt(10005) * |T|)
//
// We use fixed-point arithmetic with scale = 10^(num_digits+20) to avoid
// losing the decimal part in integer division:
//   pi * scale = 4270934400 * Q * scale^2 / (sqrt(10005 * scale^2) * |T|)

template <Arithmetic IntType>
std::string compute_pi(int num_terms, int num_digits) {
    int precision = num_digits + 20;

    auto result = binary_split<IntType>(0, num_terms);

    IntType scale    = mp::pow(IntType(10), precision);
    IntType sqrt_val = isqrt<IntType>(IntType(10005) * scale * scale);

    IntType abs_T = result.T;
    if (abs_T < 0) {
        abs_T = -abs_T;
    }

    IntType pi_int = IntType(4270934400LL) * result.Q * scale * scale
                     / (sqrt_val * abs_T);

    std::string s = pi_int.str();
    if (static_cast<int>(s.size()) > num_digits + 1) {
        s = s.substr(0, num_digits + 1);
    }
    return s.substr(0, 1) + "." + s.substr(1);
}

// RAII timer: starts on construction, call elapsed_ms() to read elapsed time.
class Timer {
  public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    [[nodiscard]] double elapsed_ms() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - start_).count();
    }

  private:
    std::chrono::high_resolution_clock::time_point start_;
};

// Each Chudnovsky term contributes ~14.18 digits (log10(640320^3/24) / 3)
// Returns the number of terms needed to compute the requested digit count.
inline int estimate_terms(int digits) {
    return static_cast<int>(std::ceil(static_cast<double>(digits) / 14.18)) + 2;
}

} // namespace pi
