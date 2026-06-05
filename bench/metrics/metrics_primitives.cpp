#include <functional>
#include <iomanip>
#include <iostream>

#include "orq.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"

using namespace orq::debug;
using namespace orq::service;
using namespace std::chrono;

using namespace COMPILED_MPC_PROTOCOL_NAMESPACE;

// Boolean shares (`BSharedVector<T>`) support: &, |, ^, !, ~, ==, >, <, >=, <=, +, -, b2a()
// Arithmetic shares (`ASharedVector<T>`) support: +, -, *, dot_product(), a2b()

size_t get_communication_rounds();
template <typename T>
void print_comm_rounds_bool(size_t size);
template <typename T>
void print_comm_rounds_arith(size_t size);
template <typename Func>
void print_time_elapsed(const std::string& name, Func&& function);
template <typename T>
void benchmark_bool(size_t size);
template <typename T>
void benchmark_arith(size_t size);

int main(const int argc, char** argv) {
    orq_init(argc, argv);
    if (argc >= 5) {
        single_cout("Test size input is ignored");
    }

    constexpr size_t min_exp = 10;
    constexpr size_t max_exp = 22;

    // Communication rounds
    for (size_t exp = min_exp; exp <= max_exp; exp++) {
        const size_t test_size = 1 << exp;
        print_comm_rounds_bool<int8_t>(test_size);
        print_comm_rounds_bool<int16_t>(test_size);
        print_comm_rounds_bool<int32_t>(test_size);
        print_comm_rounds_bool<int64_t>(test_size);
    }
    for (size_t exp = min_exp; exp <= max_exp; exp++) {
        const size_t test_size = 1 << exp;
        print_comm_rounds_arith<int8_t>(test_size);
        print_comm_rounds_arith<int16_t>(test_size);
        print_comm_rounds_arith<int32_t>(test_size);
        print_comm_rounds_arith<int64_t>(test_size);
    }
    // Runtimes
    for (size_t exp = min_exp; exp <= max_exp; exp++) {
        const size_t test_size = 1 << exp;
        benchmark_bool<int8_t>(test_size);
        benchmark_bool<int16_t>(test_size);
        benchmark_bool<int32_t>(test_size);
        benchmark_bool<int64_t>(test_size);
    }
    for (size_t exp = min_exp; exp <= max_exp; exp++) {
        const size_t test_size = 1 << exp;
        benchmark_arith<int8_t>(test_size);
        benchmark_arith<int16_t>(test_size);
        benchmark_arith<int32_t>(test_size);
        benchmark_arith<int64_t>(test_size);
    }

    return 0;
}

size_t get_communication_rounds() { return runTime->get_communicator()->getCommunicationRounds(); }

template <typename Func>
void print_comm_rounds(const std::string& name, Func&& function) {
    const size_t before = get_communication_rounds();
    std::invoke(std::forward<Func>(function));
    const size_t after = get_communication_rounds();
    single_cout(std::setw(10) << std::left << name << " took " << std::setfill('0') << std::setw(2)
                              << (after - before) << " rounds");
}

template <typename T>
void print_comm_rounds_bool(const size_t size) {
    //~ Operations on boolean shares
    BSharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Boolean vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    //~ Boolean operations
    print_comm_rounds("AND", [&] { a & b; });
    print_comm_rounds("OR", [&] { a | b; });
    print_comm_rounds("XOR", [&] { a ^ b; });
    print_comm_rounds("NEG", [&] { !a; });
    print_comm_rounds("COMPL", [&] { ~a; });
    //~ Equality & Inequality
    print_comm_rounds("EQ", [&] { a == b; });
    print_comm_rounds("NEQ", [&] { a != b; });
    //~ Comparison operations
    print_comm_rounds("GR", [&] { a > b; });
    print_comm_rounds("GE", [&] { a >= b; });
    //~ Arithmetical operations
    print_comm_rounds("ADD", [&] { a + b; });
    print_comm_rounds("SUB", [&] { a - b; });
    //~ Conversion into arithmetic share
    print_comm_rounds("CONV", [&] { a.b2a(); });
}

template <typename T>
void print_comm_rounds_arith(const size_t size) {
    //~ Operations on arithmetic shares
    ASharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Arithmetic vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    //~ Arithmetic operations
    print_comm_rounds("ADD", [&] { a + b; });
    print_comm_rounds("SUB", [&] { a - b; });
    print_comm_rounds("MULT", [&] { a* b; });
    //~ Division operations (implemented, but cause division-by-zero errors)
    // print_comm_rounds("DIV", [&] { a / b; });
    // print_comm_rounds("PUB DIV", [&] { a / static_cast<T>(9); });
    //~ Dot product operation
    print_comm_rounds("DOT 0", [&] { a.dot_product(b, 0); });
    print_comm_rounds("DOT 2", [&] { a.dot_product(b, a.size() / 2); });
    print_comm_rounds("DOT F", [&] { a.dot_product(b, a.size()); });
    //~ Conversion into boolean shares
    print_comm_rounds("CONV", [&] { a.a2b(); });
}

template <typename Func>
void print_time_elapsed(const std::string& name, Func&& function) {
    const auto start = stopwatch::get_elapsed();
    std::invoke(std::forward<Func>(function));
    const size_t end = stopwatch::get_elapsed();
    single_cout(std::setw(10) << std::left << name << " took " << (end - start) << " s");
}

template <typename T>
void benchmark_bool(const size_t size) {
    //~ Operations on boolean shares
    BSharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Boolean vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    //~ Boolean operations
    print_time_elapsed("AND", [&] { a & b; });
    print_time_elapsed("OR", [&] { a | b; });
    print_time_elapsed("XOR", [&] { a ^ b; });
    print_time_elapsed("NEG", [&] { !a; });
    print_time_elapsed("COMPL", [&] { ~a; });
    print_time_elapsed("EQ", [&] { a == b; });
    //~ Comparison operations
    print_time_elapsed("GR", [&] { a > b; });
    print_time_elapsed("GE", [&] { a >= b; });
    //~ Arithmetical operations
    print_time_elapsed("ADD", [&] { a + b; });
    print_time_elapsed("SUB", [&] { a - b; });
    //~ Conversion into arithmetic share
    print_time_elapsed("CONV", [&] { a.b2a(); });
}

template <typename T>
void benchmark_arith(const size_t size) {
    //~ Operations on arithmetic shares
    ASharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Arithmetic vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    //~ Arithmetic operations
    print_time_elapsed("ADD", [&] { a + b; });
    print_time_elapsed("SUB", [&] { a - b; });
    print_time_elapsed("MULT", [&] { a* b; });
    //~ Dot product operations
    print_time_elapsed("DOT 0", [&] { a.dot_product(b, 0); });
    print_time_elapsed("DOT 2", [&] { a.dot_product(b, a.size() / 2); });
    print_time_elapsed("DOT F", [&] { a.dot_product(b, a.size()); });
    //~ Conversion into boolean shares
    print_time_elapsed("CONV", [&] { a.a2b(); });
}

#pragma GCC diagnostic pop
