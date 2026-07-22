# Infinite Pi

Computes π (and √π) to an arbitrary number of digits using the Chudnovsky algorithm with binary splitting.

## Dependencies

| Library | Purpose | Required |
|---|---|---|
| CMake ≥ 3.25 | Build system | Yes |
| C++23 compiler (GCC ≥ 13) | Language standard | Yes |
| Boost.Multiprecision | Arbitrary-precision integers | Yes |
| Catch2 v3 | Unit test framework | Yes |
| GMP | Optional high-performance backend | No |
| Doxygen | API documentation | No |
| Sphinx + Breathe | HTML documentation site | No |

## Build

```bash
cmake -S . -B build -DPROJECT_TOPIC=PI
cmake --build build
```

### Optional: GMP backend (faster for large digit counts)

```bash
cmake -S . -B build -DPROJECT_TOPIC=PI -DPI_USE_GMP_BACKEND=ON
cmake --build build
```

### Optional: Documentation

```bash
cmake --build build --target docs    # Doxygen HTML
cmake --build build --target sphinx  # Sphinx HTML (requires Doxygen first)
```

## Usage

```bash
# Compute π to 100 digits (sequential)
./build/pi/pi 100

# Compute π using parallel binary splitting (scales to hardware_concurrency threads)
./build/pi/pi 1000 --parallel

# Compute √π to 100 digits
./build/pi/pi 100 --sqrt

# Run unit tests
./build/pi/test_pi

# Run benchmark (sequential vs parallel, and Boost vs GMP reference)
./build/pi/benchmark
```

## Algorithm

### Chudnovsky + Binary Splitting

The Chudnovsky series converges at ~14.18 decimal digits per term:

```
1/π = (√10005 / 4270934400) × Σ T(k)
```

Binary splitting avoids per-term division by accumulating P, Q, T recursively
and performing only one division at the very end. This is asymptotically faster
than naive summation for high digit counts.

### Fixed-point arithmetic

Since `cpp_int` and `mpz_int` are integers, decimal output is produced via
fixed-point scaling: multiply by `10^(digits+20)` before dividing, then insert
the decimal point in the resulting string.

### √π

`isqrt(π × 10^(2N)) ≈ √π × 10^N` — the same binary split result is reused,
only the scale factor changes.

## C++ features

| Feature | Where |
|---|---|
| C++20 concepts (`Multipliable`, `Arithmetic`) | `pi.hpp` — constrain all templates |
| `std::expected<T, E>` (C++23) | `main.cpp` — `parse_args` returns errors without exceptions |
| `std::format` (C++20) | `main.cpp` — type-safe output formatting |
| RAII | `Timer` — starts on construction, no manual start/stop |
| `constexpr noexcept` | `estimate_terms` — evaluated at compile time |
| `[[nodiscard]]` | `Timer::elapsed_ms()`, `PiComputer::pi()`, `PiComputer::sqrt_pi()` |
| `std::unique_ptr` | `PiComputer` owns the split result |
| `explicit` constructor | `PiComputer` — prevents accidental implicit conversions |
| `std::thread` (recursive) | `parallel_binary_split` — scales to `hardware_concurrency` |
| Static polymorphism | Templates + concepts instead of `virtual` (no vtable overhead) |
| `detail` namespace | Internal `parallel_split_impl` hidden from public API |
| `#ifdef` compile-time backend | Switch between `cpp_int` and `mpz_int` at build time |

## PiComputer class

`PiComputer` runs binary splitting once and caches the result via `unique_ptr`.
Both `pi()` and `sqrt_pi()` reuse the same split without recomputation:

```cpp
pi::PiComputer<BigInt> computer(100);
std::cout << computer.pi()      << "\n";  // cached split
std::cout << computer.sqrt_pi() << "\n";  // same split, different formula
```

## Validated API with std::expected

```cpp
auto result = pi::compute_pi_checked<BigInt>(terms, digits);
if (!result) {
    std::cerr << result.error() << "\n";  // descriptive error message
} else {
    std::cout << *result << "\n";
}
```

## Parallel scaling

`parallel_binary_split` recursively spawns threads up to `log2(hardware_concurrency)` depth,
using all available cores automatically. Thread creation overhead means the parallel
version is only faster beyond ~5000 digits.

## Backend comparison

| Backend | Multiplication | Sequential 10000 digits |
|---|---|---|
| `cpp_int` (default) | Schoolbook O(n²) | ~30 ms |
| `mpz_int` (GMP) | Karatsuba + FFT | ~3 ms |

The GMP backend uses the same C++ template interface — only the `using BigInt = ...`
line changes. The algorithm is backend-agnostic by design.

## Reference comparison

Build with `-DBUILD_REFERENCE=ON` to compile `gmp-chudnovsky` (the reference C
implementation from gmplib.org). The benchmark will automatically compare timings.
