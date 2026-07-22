# Infinite Pi

This project computes pi to an arbitrary number of digits using the Chudnovsky algorithm with binary splitting.

## How it works

The Chudnovsky algorithm is a fast series for computing pi. Each term gives about 14 new digits. Binary splitting is a technique to compute the sum more efficiently — instead of dividing at every step, we only do one division at the end.

For large numbers I used the Boost.Multiprecision library (`cpp_int`), because normal `int` or `double` can only hold around 15-19 digits.

## Build

```bash
cmake -S . -B build
cmake --build build
```

Requires: CMake, a C++23 compiler, Boost

## Usage

```bash
# compute pi to 100 digits
./build/pi/pi 100

# use parallel computation
./build/pi/pi 1000 --parallel

# compute sqrt(pi) instead
./build/pi/pi 100 --sqrt

# run unit tests
./build/pi/test_pi

# run benchmark (sequential vs parallel)
./build/pi/benchmark
```

## What I implemented

**Basic task**
- `binary_split(a, b)` — recursive binary splitting, accumulates P, Q, T values
- `isqrt(n)` — integer square root using Newton's method, needed because `std::sqrt` loses precision on big numbers
- `compute_pi(terms, digits)` — puts everything together, uses fixed-point arithmetic to avoid losing decimal digits
- `estimate_terms(digits)` — calculates how many terms are needed for the requested digit count

**Improvement task**
- `estimate_terms` — digit estimation so you can say "give me 1000 digits" instead of guessing term count
- `compute_sqrt_pi` — computes sqrt(pi) to the same number of digits
- benchmark against the reference C implementation (gmp-chudnovsky)

**Advanced task**
- `parallel_binary_split` — splits the work between two threads using `std::thread`
- speedup analysis in benchmark.cpp

## C++ features used

- C++20 concepts (`Multipliable`, `Arithmetic`) to constrain template parameters
- RAII for the Timer class
- `[[nodiscard]]` attribute
- `std::thread` for parallelism
- Static polymorphism with templates instead of virtual functions
- `static_assert` in tests for compile-time checks

## Notes

For small digit counts (under ~5000), the parallel version is actually slower because opening a thread takes more time than the computation itself. The parallel version starts to win around 10000+ digits.

The reference implementation `gmp-chudnovsky` uses the GMP library which has hand-optimized assembly for big integer multiplication. Our Boost implementation is simpler but slower for very large digit counts.
