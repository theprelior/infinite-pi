option(BUILD_REFERENCE "Build the reference code (gmp-chudnovsky)" OFF)

if(BUILD_REFERENCE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)

  file(DOWNLOAD https://gmplib.org/download/misc/gmp-chudnovsky.c
       "${CMAKE_CURRENT_BINARY_DIR}/gmp-chudnovsky.c" SHOW_PROGRESS)

  add_executable(gmp-chudnovsky "${CMAKE_CURRENT_BINARY_DIR}/gmp-chudnovsky.c")
  target_include_directories(gmp-chudnovsky PRIVATE ${GMP_INCLUDE_DIRS})
  target_link_libraries(gmp-chudnovsky PRIVATE PkgConfig::GMP m)
  #[[
	gmp-chudnovsky <digits> <1 if you want to print pi>
	gmp-chudnovsky 100 1
	gmp-chudnovsky 1000 1
	]]
endif()

option(PI_USE_GMP_BACKEND "Use boost::multiprecision::mpz_int (GMP) instead of cpp_int" OFF)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

if(PI_USE_GMP_BACKEND)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)
endif()

add_executable(pi main.cpp)
target_compile_features(pi PRIVATE "cxx_std_${PROJECT_CXX_STANDARD}")
target_link_libraries(pi PRIVATE Boost::boost Threads::Threads)

find_package(Catch2 3 REQUIRED)

add_executable(test_pi test_pi.cpp)
target_compile_features(test_pi PRIVATE "cxx_std_${PROJECT_CXX_STANDARD}")
target_link_libraries(test_pi PRIVATE Boost::boost Threads::Threads Catch2::Catch2WithMain)

add_executable(benchmark benchmark.cpp)
target_compile_features(benchmark PRIVATE "cxx_std_${PROJECT_CXX_STANDARD}")
target_link_libraries(benchmark PRIVATE Boost::boost Threads::Threads)

if(PI_USE_GMP_BACKEND)
  foreach(tgt pi test_pi benchmark)
    target_compile_definitions(${tgt} PRIVATE PI_USE_GMP_BACKEND)
    target_link_libraries(${tgt} PRIVATE PkgConfig::GMP)
  endforeach()
endif()
