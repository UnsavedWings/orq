#include "orq.h"

using namespace orq::debug;
using namespace orq::service;
using namespace COMPILED_MPC_PROTOCOL_NAMESPACE;

size_t get_communication_rounds() { return runTime->get_communicator()->getCommunicationRounds(); }

void benchmark(int pid);
void _benchmark(int pid, int test_size);
void print_comm_rounds(int pid, int test_size);

// command
// mpirun -np 3 ./micro_randomness 1 1 8192 $ROWS

int main(const int argc, char** argv) {
    orq_init(argc, argv);
    const auto pID = runTime->getPartyID();
    int test_size = 1000000;
    if (argc >= 5) {
        test_size = atoi(argv[4]);
    }

    benchmark(pID);
    print_comm_rounds(pID, test_size);

    return 0;
}

void benchmark(const int pid) {
    constexpr int min_exp = 9;
    constexpr int max_exp = 24;

    for (int b = min_exp; b <= max_exp; b++) {
        const size_t total_size = 1 << b;

        single_cout(std::endl);
        _benchmark(pid, total_size);
    }
}

void _benchmark(const int pid, const int test_size) {
    orq::Vector<int> local(test_size);
    orq::Vector<int> common(test_size);

    // stop timer
    stopwatch::timepoint(std::format("Start total size={}", test_size));

    // local randomness
    runTime->populateLocalRandom(local);

    // stop timer
    stopwatch::timepoint("Local Randomness");

    // common randomness
    const std::set<int> group = runTime->getGroups()[0];
    if (group.contains(pid)) {
        runTime->populateCommonRandom(common, group);
    }

    // stop timer
    stopwatch::timepoint("Common Randomness");
}

void print_comm_rounds(const int pid, const int test_size) {
    orq::Vector<int> local(test_size);
    orq::Vector<int> common(test_size);

    single_cout("CR before local rand-gen: " << get_communication_rounds());

    // local randomness
    runTime->populateLocalRandom(local);

    single_cout("CR after local rand-gen: " << get_communication_rounds());

    // common randomness
    const std::set<int> group = runTime->getGroups()[0];
    if (group.contains(pid)) {
        runTime->populateCommonRandom(common, group);
    }

    single_cout("CR after common rand-gen: " << get_communication_rounds());
}