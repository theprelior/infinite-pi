#pragma once

/// @file pi.hpp
/// @brief Chudnovsky algorithm with binary splitting for computing pi to arbitrary precision.
///
/// Uses Boost.Multiprecision (cpp_int) for arbitrary-precision integers.
/// All templates are constrained with C++20 concepts.

#include <boost/multiprecision/cpp_int.hpp>

#include <chrono>
#include <cmath>
#include <concepts>
#include <memory>
#include <string>
#include <thread>

/// @namespace pi
/// @brief Contains all types and functions for arbitrary-precision pi computation.
namespace pi {

namespace mp = boost::multiprecision;

/// @concept Multipliable
/// @brief Requires that two values of type T can be multiplied and the result is convertible to T.
template <typename T>
concept Multipliable = requires(T a, T b) {
    { a * b } -> std::convertible_to<T>;
};

/// @concept Arithmetic
/// @brief Requires that T supports addition, subtraction, and multiplication.
///
/// Used to constrain all template parameters in this library.
/// Any type satisfying Arithmetic can be used as the integer type (e.g. cpp_int, int).
template <typename T>
concept Arithmetic = Multipliable<T> && requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { a - b } -> std::convertible_to<T>;
};

/// @brief Holds the three accumulated values from binary splitting.
///
/// Binary splitting computes the Chudnovsky series by accumulating P, Q, T
/// recursively. Only one division is needed at the very end.
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
template <Arithmetic IntType>
struct SplitResult {
    IntType P; ///< Numerator accumulator
    IntType Q; ///< Denominator accumulator
    IntType T; ///< Series term accumulator
};

/// @brief Recursively computes the Chudnovsky binary splitting over [a, b).
///
/// Base case (b - a == 1): computes p(k), q(k), t(k) directly from the Chudnovsky formula.
/// Recursive case: splits into [a, mid) and [mid, b), merges with:
///   - P(a,b) = P(a,m) * P(m,b)
///   - Q(a,b) = Q(a,m) * Q(m,b)
///   - T(a,b) = T(a,m) * Q(m,b) + P(a,m) * T(m,b)
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
/// @param a Start index (inclusive).
/// @param b End index (exclusive).
/// @return SplitResult containing accumulated P, Q, T for the range [a, b).
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

/// @brief Parallel version of binary_split using std::thread.
///
/// Computes the left half [a, mid) in a separate thread while the main thread
/// handles the right half [mid, b). Both results are merged after joining.
///
/// For small ranges (b - a <= 1) falls back to binary_split to avoid
/// the overhead of spawning a thread for trivial work.
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
/// @param a Start index (inclusive).
/// @param b End index (exclusive).
/// @return SplitResult containing accumulated P, Q, T for the range [a, b).
template <Arithmetic IntType>
SplitResult<IntType> parallel_binary_split(int a, int b) {
    if (b - a <= 1) {
        return binary_split<IntType>(a, b);
    }

    int mid = (a + b) / 2;
    SplitResult<IntType> left, right;

    std::thread t([&]() { left = binary_split<IntType>(a, mid); });
    right = binary_split<IntType>(mid, b);
    t.join();

    return {
        left.P * right.P,
        left.Q * right.Q,
        left.T * right.Q + left.P * right.T
    };
}

/// @brief Integer square root using Newton's method.
///
/// Returns floor(sqrt(n)). Uses Newton iteration: x_{i+1} = (x + n/x) / 2.
/// Converges when next >= x (i.e. no further improvement possible).
///
/// Needed because std::sqrt works on double (~15 significant digits),
/// which is insufficient for high-precision pi computation.
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
/// @param n The value to take the square root of.
/// @return floor(sqrt(n))
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

/// @brief Converts a precomputed SplitResult into a pi string.
///
/// Uses fixed-point arithmetic with scale = 10^(num_digits+20) to preserve
/// decimal precision during integer division:
///   pi * scale = 4270934400 * Q * scale^2 / (sqrt(10005 * scale^2) * |T|)
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
/// @param result Precomputed binary split result.
/// @param num_digits Number of decimal digits to return.
/// @return Pi as a string, e.g. "3.14159..."
template <Arithmetic IntType>
std::string compute_pi_from_split(const SplitResult<IntType>& result, int num_digits) {
    int precision = num_digits + 20;
    IntType scale    = mp::pow(IntType(10), precision);
    IntType sqrt_val = isqrt<IntType>(IntType(10005) * scale * scale);
    IntType abs_T    = result.T < 0 ? -result.T : result.T;
    IntType pi_int   = IntType(4270934400LL) * result.Q * scale * scale
                       / (sqrt_val * abs_T);
    std::string s = pi_int.str();
    if (static_cast<int>(s.size()) > num_digits + 1) {
        s = s.substr(0, num_digits + 1);
    }
    return s.substr(0, 1) + "." + s.substr(1);
}

/// @brief Computes pi to num_digits decimal places.
///
/// Runs binary_split internally, then calls compute_pi_from_split.
/// For reuse of the split result, prefer PiComputer.
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
/// @param num_terms Number of Chudnovsky terms to use.
/// @param num_digits Number of decimal digits to return.
/// @return Pi as a string, e.g. "3.14159..."
template <Arithmetic IntType>
std::string compute_pi(int num_terms, int num_digits) {
    auto result = binary_split<IntType>(0, num_terms);
    return compute_pi_from_split<IntType>(result, num_digits);
}

/// @brief Converts a precomputed SplitResult into a sqrt(pi) string.
///
/// Computes pi × 10^(2*N) as a big integer, then applies isqrt:
///   isqrt(pi × 10^(2*N)) ≈ sqrt(pi) × 10^N
/// where N = num_digits + 20.
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
/// @param result Precomputed binary split result.
/// @param num_digits Number of decimal digits to return.
/// @return sqrt(pi) as a string, e.g. "1.77245..."
template <Arithmetic IntType>
std::string compute_sqrt_pi_from_split(const SplitResult<IntType>& result, int num_digits) {
    int N            = num_digits + 20;
    IntType scale    = mp::pow(IntType(10), 2 * N);
    IntType sqrt_val = isqrt<IntType>(IntType(10005) * scale * scale);
    IntType abs_T    = result.T < 0 ? -result.T : result.T;
    IntType pi_big   = IntType(4270934400LL) * result.Q * scale * scale
                       / (sqrt_val * abs_T);
    IntType sqrt_pi_int = isqrt<IntType>(pi_big);
    std::string s = sqrt_pi_int.str();
    if (static_cast<int>(s.size()) > num_digits + 1) {
        s = s.substr(0, num_digits + 1);
    }
    return s.substr(0, 1) + "." + s.substr(1);
}

/// @brief Computes sqrt(pi) to num_digits decimal places.
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
/// @param num_terms Number of Chudnovsky terms to use.
/// @param num_digits Number of decimal digits to return.
/// @return sqrt(pi) as a string, e.g. "1.77245..."
template <Arithmetic IntType>
std::string compute_sqrt_pi(int num_terms, int num_digits) {
    auto result = binary_split<IntType>(0, num_terms);
    return compute_sqrt_pi_from_split<IntType>(result, num_digits);
}

/// @brief RAII high-resolution timer.
///
/// Starts automatically on construction. Call elapsed_ms() at any point
/// to read the elapsed time in milliseconds. No manual start/stop needed.
class Timer {
  public:
    /// @brief Starts the timer.
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    /// @brief Returns elapsed time in milliseconds since construction.
    /// @return Elapsed milliseconds as a double.
    [[nodiscard]] double elapsed_ms() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - start_).count();
    }

  private:
    std::chrono::high_resolution_clock::time_point start_;
};

/// @brief Estimates the number of Chudnovsky terms needed for a given digit count.
///
/// Each term contributes approximately 14.18 digits (= log10(640320^3/24) / 3).
/// Adds 2 extra terms as a safety margin.
///
/// @param digits Desired number of decimal digits.
/// @return Number of terms required.
inline int estimate_terms(int digits) {
    return static_cast<int>(std::ceil(static_cast<double>(digits) / 14.18)) + 2;
}

/// @brief High-level wrapper that owns the binary split result and exposes pi() and sqrt_pi().
///
/// Runs binary_split once on construction and stores the result via unique_ptr.
/// Both pi() and sqrt_pi() reuse the same split — no recomputation needed.
///
/// @tparam IntType An integer type satisfying the Arithmetic concept.
///
/// Example:
/// @code
/// pi::PiComputer<BigInt> computer(100);
/// std::cout << computer.pi()      << "\n";
/// std::cout << computer.sqrt_pi() << "\n";
/// @endcode
template <Arithmetic IntType>
class PiComputer {
  public:
    /// @brief Constructs a PiComputer and runs binary splitting for num_digits.
    /// @param num_digits Number of decimal digits to compute.
    explicit PiComputer(int num_digits)
        : num_digits_(num_digits),
          num_terms_(estimate_terms(num_digits)),
          split_(std::make_unique<SplitResult<IntType>>(
              binary_split<IntType>(0, num_terms_))) {}

    /// @brief Returns pi to num_digits decimal places.
    /// @return Pi as a string, e.g. "3.14159..."
    std::string pi()      const { return compute_pi_from_split<IntType>(*split_, num_digits_); }

    /// @brief Returns sqrt(pi) to num_digits decimal places.
    /// @return sqrt(pi) as a string, e.g. "1.77245..."
    std::string sqrt_pi() const { return compute_sqrt_pi_from_split<IntType>(*split_, num_digits_); }

  private:
    int                                   num_digits_;
    int                                   num_terms_;
    std::unique_ptr<SplitResult<IntType>> split_; ///< Owned split result, freed on destruction.
};

} // namespace pi
