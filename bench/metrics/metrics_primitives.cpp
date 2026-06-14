#include <functional>
#include <iomanip>
#include <iostream>

#include "orq.h"
#include "util.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"

using namespace orq::debug;
using namespace orq::service;
using namespace std::chrono;

using namespace COMPILED_MPC_PROTOCOL_NAMESPACE;

template <typename T>
void count_comm_rounds_bool(size_t size);
template <typename T>
void count_comm_rounds_arith(size_t size);
// runtime benchmarks
template <typename T>
void benchmark_bool(size_t size);
template <typename T>
void benchmark_arith(size_t size);
// test functions
void run_simple_test();
void run_comm_rounds_test();
void run_benchmark_test();

int main(const int argc, char** argv) {
    orq_init(argc, argv);
    // size_t test_size = 128;
    // if (argc >= 5) {
    //     test_size = atoi(argv[4]);
    // }

    run_comm_rounds_test();

    return 0;
}


void run_benchmark_test() {

    constexpr size_t min_exp = 25;
    constexpr size_t max_exp = 25;

    const auto local_iterate = [&](const std::function<void(const size_t&)>& func) {
        iterate(min_exp, max_exp, func);
    };

    // Runtimes of operations on boolean shares
    local_iterate(benchmark_bool<int8_t>);
    local_iterate(benchmark_bool<int16_t>);
    local_iterate(benchmark_bool<int32_t>);
    local_iterate(benchmark_bool<int64_t>);

    // Runtimes of operations on arithmetic shares
    local_iterate(benchmark_arith<int8_t>);
    local_iterate(benchmark_arith<int16_t>);
    local_iterate(benchmark_arith<int32_t>);
    local_iterate(benchmark_arith<int64_t>);
}

void run_comm_rounds_test() {

    constexpr size_t min_exp = 7;
    constexpr size_t max_exp = 7;

    const auto local_iterate = [&](const std::function<void(const size_t&)>& func) {
        iterate(min_exp, max_exp, func);
    };

    // Communication rounds of operations on boolean shares
    local_iterate(count_comm_rounds_bool<int8_t>);
    local_iterate(count_comm_rounds_bool<int16_t>);
    local_iterate(count_comm_rounds_bool<int32_t>);
    local_iterate(count_comm_rounds_bool<int64_t>);

    // Communication round of operations on arithmetic shares
    local_iterate(count_comm_rounds_arith<int8_t>);
    local_iterate(count_comm_rounds_arith<int16_t>);
    local_iterate(count_comm_rounds_arith<int32_t>);
    local_iterate(count_comm_rounds_arith<int64_t>);
}

void run_simple_test() {
    constexpr size_t min_exp = 10;
    constexpr size_t max_exp = 22;

    iterate(min_exp, max_exp,
            [](const size_t& size) {
                BSharedVector<int8_t> a(size), b(size);
                single_cout(std::endl
                            << "Boolean vector " << size << " x "
                            << std::numeric_limits<std::make_unsigned_t<int8_t>>::digits << "b");
                print_comm_rounds("AND", [&] { static_cast<void>(a & b); });
            });
}

template <typename T>
void count_comm_rounds_bool(size_t size) {
    // Operations on boolean shares
    BSharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Boolean vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    //~ Boolean operations
    const auto bool_ops = make_bool_ops(a, b);

    for (const auto& [name, func] : bool_ops) {
        print_comm_rounds(name, func);
    }
}

template <typename T>
void count_comm_rounds_arith(size_t size) {
    // Operations on arithmetic shares
    ASharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Arithmetic vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    const auto arith_ops = make_arith_ops(a, b);

    for (const auto& [name, func] : arith_ops) {
        print_comm_rounds(name, func);
    }
}

template <typename T>
void benchmark_bool(const size_t size) {
    // Operations on boolean shares
    BSharedVector<T> a(size), b(size);


    single_cout(std::endl
                << "Boolean vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    const auto bool_ops = make_bool_ops(a, b);

    for (const auto& [name, func] : bool_ops) {
        print_time_elapsed(name, func);
    }
}

template <typename T>
void benchmark_arith(const size_t size) {
    // Operations on arithmetic shares
    ASharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Arithmetic vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    const auto arith_ops = make_arith_ops(a, b);

    for (const auto& [name, func] : arith_ops) {
        print_time_elapsed(name, func);
    }
}

#pragma GCC diagnostic pop
