#include "orq.h"
#include "util.h"
#include "profiling/stopwatch.h"

using namespace orq::debug;
using namespace orq::service;
using namespace COMPILED_MPC_PROTOCOL_NAMESPACE;

#include <unistd.h>
// command
// mpirun -np 3 ./micro_shuffling 1 1 8192 $ROWS

template <typename T>
void run_benchmark(size_t test_size);
template <typename T>
void run_comm_rounds_test(size_t test_size);


int main(const int argc, char** argv) {
    orq_init(argc, argv);
    // int test_size = 128;
    // int num_columns = 2;
    // if (argc >= 5) {
    //     // test_size = atoi(argv[4]);
    // }
    // if (argc >= 6) {
    //     num_columns = atoi(argv[5]);
    // }

    run_comm_rounds_test<int32_t>(2 << 10);

    return 0;
}

template <typename T>
void run_benchmark(const size_t test_size) {
    orq::Vector<int32_t> v(test_size);
    runTime->populateLocalRandom(v);

    BSharedVector<T> b = secret_share_b(v, 0);

    // start timer
    stopwatch::timepoint("Start");
    stopwatch::profile_init();

    b.shuffle();

    // stop timer
    stopwatch::timepoint("Shuffle");
    stopwatch::profile_done();

    runTime->print_statistics();
    runTime->print_communicator_statistics();
}


template <typename T>
void run_comm_rounds_test(size_t test_size) {
    orq::Vector<int32_t> v(test_size);
    runTime->populateLocalRandom(v);

    BSharedVector<T> b = secret_share_b(v, 0);

    print_comm_rounds("shuffle", [&b] { b.shuffle(); });
}