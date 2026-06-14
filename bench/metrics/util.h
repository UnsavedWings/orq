#pragma once
#include <mpi.h>

#include "orq.h"

using namespace orq::debug;
using namespace orq::service;
using namespace COMPILED_MPC_PROTOCOL_NAMESPACE;

inline void pause_for_keypress() {
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (runTime->getPartyID() == 0) {
        std::cout << std::endl << "Press Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Synchronize all processes after the pause
    MPI_Barrier(MPI_COMM_WORLD);
}

inline void iterate(const size_t min_exp, const size_t max_exp,
                    const std::function<void(const size_t&)>& func) {
    for (size_t exp = min_exp; exp <= max_exp; exp++) {
        func(1 << exp);
    }
}

inline void iterateWithPause(const size_t min_exp, const size_t max_exp,
                             const std::function<void(const size_t&)>& func) {
    for (size_t exp = min_exp; exp <= max_exp; exp++) {
        func(1 << exp);
    }
    pause_for_keypress();
}

inline size_t get_comm_rounds() { return runTime->get_communicator()->getCommunicationRounds(); }

template <typename Func>
size_t get_comm_rounds(Func&& function) {
    const auto start = get_comm_rounds();
    std::invoke(std::forward<Func>(function));
    const auto end = get_comm_rounds();
    return end - start;
}

template <typename Func>
void print_comm_rounds(const std::string& name, Func&& function) {
    const size_t rounds = get_comm_rounds(std::forward<Func>(function));
    single_cout(std::setw(10) << std::left << name << " took " << std::right << std::setfill(' ')
                              << std::setw(3) << rounds << " rounds");
}

template <typename Func>
void print_time_elapsed(const std::string& name, Func&& function) {
    const auto start = stopwatch::get_elapsed();
    std::invoke(std::forward<Func>(function));
    const auto end = stopwatch::get_elapsed();
    single_cout(std::setw(10) << std::left << name << " took " << (end - start) << " s");
}

template <typename T>
void populate_secret_vectors_for_division(BSharedVector<T>& a, BSharedVector<T>& b) {
    const size_t test_size = a.size();
    assert(test_size == b.size());

    using unsigned_type_t = std::make_unsigned_t<T>;

    const auto m = static_cast<double>(std::numeric_limits<T>::max());
    auto scale = static_cast<T>(sqrt(m));

    // Generate reasonable division test values
    Vector<unsigned_type_t> unsigned_vector(test_size);
    runTime->populateLocalRandom(unsigned_vector);
    unsigned_vector = (unsigned_vector % (scale - 1)) + 1;

    Vector<unsigned_type_t> q(test_size);
    runTime->populateLocalRandom(q);
    q = (q % (scale - 1)) + 1;

    Vector<T> open_b_vector(test_size);
    open_b_vector = unsigned_vector;

    Vector<T> open_a_vector(test_size);
    open_a_vector = unsigned_vector * q;

    a = secret_share_a(open_a_vector, 0);
    b = secret_share_b(open_b_vector, 0);
}

template <typename T>
auto make_bool_ops(BSharedVector<T>& a, BSharedVector<T>& b) {
    // Boolean shares (`BSharedVector<T>`) support: &, |, ^, !, ~, ==, >, <, >=, <=, +, -, /, b2a()
    populate_secret_vectors_for_division(a, b);
    return std::vector<std::pair<std::string, std::function<void()>>>{
            {"AND", [&] { a & b; }},
            {"OR", [&] { a | b; }},
            {"XOR", [&] { a ^ b; }},
            {"NEG", [&] { !a; }},
            {"COMPLEMENT", [&] { ~a; }},
            {"EQ", [&] { a == b; }},
            {"NEQ", [&] { a != b; }},
            {"GR", [&] { a > b; }},
            {"GE", [&] { a >= b; }},
            {"COMPARE", [&] { a > b; }},
            {"ADD", [&] { a + b; }},
            {"SUB", [&] { a - b; }},
            {"DIV", [&] { a / b; }},
            {"CONV", [&] { a.b2a(); }},
        };
}

template <typename T>
auto make_arith_ops(ASharedVector<T>& a, ASharedVector<T>& b) {

    // Arithmetic shares (`ASharedVector<T>`) support: +, -, *, /, dot_product(), a2b()
    return std::vector<std::pair<std::string, std::function<void()>>>{
            {"ADD", [&] { a + b; }},
            {"SUB", [&] { a - b; }},
            {"MULT", [&] { a* b; }},
            {"MULT PUB", [&] { a * static_cast<T>(9); }},
            {"DIV", [&] { a / static_cast<T>(9); }},
            {"PRIVAT DIV", [&] { a / b; }},
            {"DOT 1", [&] { a.dot_product(b, 1); }},
            {"DOT H", [&] { a.dot_product(b, a.size() / 2); }},
            {"DOT F", [&] { a.dot_product(b, a.size()); }},
            {"CONV", [&] { a.a2b(); }}};
}