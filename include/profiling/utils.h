#pragma once

#include <sys/time.h>

#include "debug/orq_debug.h"

using sec = std::chrono::duration<float, std::chrono::seconds::period>;

#define PROFILE(EXPR, NAME)                                                         \
    {                                                                               \
        auto t1 = std::chrono::steady_clock::now();                                 \
        (EXPR);                                                                     \
        auto t2 = std::chrono::steady_clock::now();                                 \
        elapsed = sec(t2 - t1).count();                                             \
        single_cout(std::setw(30) << std::left << (NAME " " #EXPR) << "\telapsed\t" \
                                  << std::setw(10) << elapsed);                     \
    }

namespace orq::benchmarking::utils {

static void print_bin(const int num1, const int num2, const bool add_line) {
    const std::bitset<32> x(num1);
    const std::bitset<32> y(num2);
    std::cout << x << "\t\t" << y << "\t\t";

    if (add_line) {
        std::cout << '\n';
    }
}

template <typename... T>
static void timeTest(void(func)(T...), const std::string& name, const int batch_nums,
                     const int size, T... args) {
    timeval begin{}, end{};

    // start timer
    gettimeofday(&begin, nullptr);
    for (int i = 0; i < batch_nums; ++i) {
        func(args...);
    }

    // stop timer
    gettimeofday(&end, nullptr);
    const auto seconds = static_cast<double>(end.tv_sec - begin.tv_sec);
    const auto micro = static_cast<double>(end.tv_usec - begin.tv_usec);
    const double elapsed = seconds + micro * 1e-6;

    printf("\n%s \tpassed in:\t\t%f\t\t\tThroughput:\t\t%dk\n", name.c_str(), elapsed,
           static_cast<int>(size * batch_nums / elapsed / 1000));
}

template <typename T>
static int duration_to_ms(std::chrono::time_point<T> a, std::chrono::time_point<T> b) {
    return duration_cast<std::chrono::milliseconds>(b - a).count();
}

}  // namespace orq::benchmarking::utils
