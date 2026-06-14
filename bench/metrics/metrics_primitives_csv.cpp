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

template <typename Func>
void count_comm_rounds_to_csv_line(const std::string& sharing, size_t bytes, size_t exponent,
                                   const std::string& operation_name, Func&& function);
template <typename T>
void print_comm_rounds_bool_ops_as_csv(size_t size);
template <typename T>
void print_comm_rounds_arith_ops_as_csv(size_t size);

// runtime benchmarks
template <typename T>
void benchmark_bool(size_t size);
template <typename T>
void benchmark_arith(size_t size);

// test functions
void run_full_test();
void run_simple_test();

int main(const int argc, char** argv) {
    orq_init(argc, argv);
    if (argc >= 5) {
        single_cout("Test size input is ignored");
    }

    run_simple_test();

    return 0;
}

void run_full_test() {
    constexpr size_t min_exp = 10;
    constexpr size_t max_exp = 22;

    const auto local_iterate = [&](const std::function<void(const size_t&)>& func) {
        iterate(min_exp, max_exp, func);
    };

    // Communication rounds of operations on boolean shares
    local_iterate(print_comm_rounds_bool_ops_as_csv<int8_t>);
    local_iterate(print_comm_rounds_bool_ops_as_csv<int16_t>);
    local_iterate(print_comm_rounds_bool_ops_as_csv<int32_t>);
    local_iterate(print_comm_rounds_bool_ops_as_csv<int64_t>);
    // Communication round of operations on arithmetic shares
    local_iterate(print_comm_rounds_arith_ops_as_csv<int8_t>);
    local_iterate(print_comm_rounds_arith_ops_as_csv<int16_t>);
    local_iterate(print_comm_rounds_arith_ops_as_csv<int32_t>);
    local_iterate(print_comm_rounds_arith_ops_as_csv<int64_t>);
    // Runtimes of operations on boolean shares
    local_iterate(benchmark_bool<int8_t>);
    local_iterate(benchmark_bool<int16_t>);
    local_iterate(benchmark_bool<int32_t>);
    local_iterate(benchmark_bool<int64_t>);
    // Runtimes of operations on arithmetic shares
    local_iterate(print_comm_rounds_arith_ops_as_csv<int8_t>);
    local_iterate(print_comm_rounds_arith_ops_as_csv<int16_t>);
    local_iterate(print_comm_rounds_arith_ops_as_csv<int32_t>);
    local_iterate(print_comm_rounds_arith_ops_as_csv<int64_t>);
}

void run_simple_test() {}

template <typename Func>
void count_comm_rounds_to_csv_line(const std::string& sharing, const size_t bytes,
                                   const size_t exponent, const std::string& operation_name,
                                   Func&& function) {
    const size_t rounds = get_comm_rounds(std::forward<Func>(function));

    // putRound(sharing, bytes, exponent, ObliviousOperation.AND, 4);
    single_cout(sharing << "," << bytes << "," << exponent << "," << operation_name << ","
                        << rounds);
}

template <typename T>
void print_comm_rounds_bool_ops_as_csv(const size_t size) {
    //~ Operations on boolean shares
    BSharedVector<T> a(size), b(size);

    populate_secret_vectors_for_division(a, b);

    single_cout(std::endl << "exponent = " << static_cast<size_t>(std::countr_zero(size)) << ";");

    //~ Boolean operations
    const std::vector<std::pair<std::string, std::function<void()>>> bool_ops = {
        {"AND", [&] { a & b; }},
        {"OR", [&] { a | b; }},
        {"XOR", [&] { a ^ b; }},
        {"NEG", [&] { !a; }},
        {"COMPLEMENT", [&] { ~a; }},
        {"EQ", [&] { a == b; }},
        {"NEQ", [&] { a != b; }},
        //~ Comparison operations
        {"GR", [&] { a > b; }},
        {"GE", [&] { a >= b; }},
        {"COMPARE", [&] { a > b; }},
        //~ Arithmetical operations
        {"ADD", [&] { a + b; }},
        {"SUB", [&] { a - b; }},
        {"DIV", [&]{ a / b; }},
        //~ Conversion into arithmetic share
        {"CONV", [&] { a.b2a(); }}};

    for (const auto& [name, func] : bool_ops) {
        count_comm_rounds_to_csv_line("bool", sizeof(T),
                                      static_cast<size_t>(std::countr_zero(size)), name, func);
    }
}

template <typename T>
void print_comm_rounds_arith_ops_as_csv(const size_t size) {
    //~ Operations on arithmetic shares
    ASharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Arithmetic vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    //~ Arithmetic operations
    const std::vector<std::pair<std::string, std::function<void()>>> arith_ops = {
        {"ADD", [&] { a + b; }},
        {"SUB", [&] { a - b; }},
        {"MULT", [&] { a* b; }},
        {"DIV", [&] { a / static_cast<T>(9); }},
        {"DOT 1", [&] { a.dot_product(b, 1); }},
        {"DOT H", [&] { a.dot_product(b, a.size() / 2); }},
        {"DOT F", [&] { a.dot_product(b, a.size()); }},
        {"CONV", [&] { a.a2b(); }}};

    for (const auto& [name, func] : arith_ops) {
        count_comm_rounds_to_csv_line("arith", sizeof(T),
                                      static_cast<size_t>(std::countr_zero(size)), name, func);
    }
}

template <typename T>
void benchmark_bool(const size_t size) {
    //~ Operations on boolean shares
    BSharedVector<T> a(size), b(size);

    populate_secret_vectors_for_division(a, b);

    single_cout(std::endl
                << "Boolean vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    //~ Boolean operations
    const std::vector<std::pair<std::string, std::function<void()>>> bool_ops = {
        {"AND", [&] { a & b; }},
        {"OR", [&] { a | b; }},
        {"XOR", [&] { a ^ b; }},
        {"NEG", [&] { !a; }},
        {"COMPLEMENT", [&] { ~a; }},
        {"EQ", [&] { a == b; }},
        {"NEQ", [&] { a != b; }},
        //~ Comparison operations
        {"GR", [&] { a > b; }},
        {"GE", [&] { a >= b; }},
        {"COMPARE", [&] { a > b; }},
        //~ Arithmetical operations
        {"ADD", [&] { a + b; }},
        {"SUB", [&] { a - b; }},
        {"DIV", [&]{ a / b; }},
        //~ Conversion into arithmetic share
        {"CONV", [&] { a.b2a(); }}};

    for (const auto& [name, func] : bool_ops) {
        print_time_elapsed(name, func);
    }
}

template <typename T>
void benchmark_arith(const size_t size) {
    //~ Operations on arithmetic shares
    ASharedVector<T> a(size), b(size);

    single_cout(std::endl
                << "Arithmetic vector " << size << " x "
                << std::numeric_limits<std::make_unsigned_t<T>>::digits << "b");

    //~ Arithmetic operations
    const std::vector<std::pair<std::string, std::function<void()>>> arith_ops = {
        {"ADD", [&] { a + b; }},
        {"SUB", [&] { a - b; }},
        {"MULT", [&] { a* b; }},
        {"DIV", [&] { a / static_cast<T>(9); }},
        {"DOT 1", [&] { a.dot_product(b, 1); }},
        {"DOT H", [&] { a.dot_product(b, a.size() / 2); }},
        {"DOT F", [&] { a.dot_product(b, a.size()); }},
        {"CONV", [&] { a.a2b(); }}};

    for (const auto& [name, func] : arith_ops) {
        print_time_elapsed(name, func);
    }
}

#pragma GCC diagnostic pop
