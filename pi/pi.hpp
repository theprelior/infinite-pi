#pragma once

#include <boost/multiprecision/cpp_int.hpp>

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

} // namespace pi
