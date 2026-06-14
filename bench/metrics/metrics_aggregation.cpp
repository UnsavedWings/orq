#include "orq.h"
#include "util.h"

using namespace orq::debug;
using namespace orq::service;

using namespace COMPILED_MPC_PROTOCOL_NAMESPACE;

void benchmark(size_t test_size);
void count_rounds(size_t test_size);

int main(const int argc, char** argv) {
    orq_init(argc, argv);
    const auto pID = runTime->getPartyID();
    // int test_size = 128;
    // if (argc >= 5) {
    //     test_size = atoi(argv[4]);
    // }

    single_cout("Default bitwidth: " << DEFAULT_BITWIDTH);

    iterate(7, 15, [&](const size_t size) {
        single_cout("size: " << size);
        count_rounds(size);
    });

    return 0;
}

void benchmark(const size_t test_size) {
    std::vector<std::string> schema = {"[SEL]", "DATA", "[DATA]", "SUM", "[MAX]", "[MIN]"};
    std::vector<orq::Vector<int>> data(schema.size(), test_size);
    EncodedTable<int> table = secret_share(data, schema);

    // start timer
    timeval begin, end;
    long seconds, micro;
    double elapsed;
    gettimeofday(&begin, 0);

    using A = ASharedVector<int>;

    table.aggregate({"[SEL]"}, {{"DATA", "SUM", orq::aggregators::sum<A>}});

    // stop timer
    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec;
    micro = end.tv_usec - begin.tv_usec;
    elapsed = seconds + micro * 1e-6;
    single_cout("SUM_AGGREGATION:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed);

    gettimeofday(&begin, 0);

    using B = BSharedVector<int>;

    table.aggregate({"[SEL]"}, {
                                   {"[DATA]", "[MIN]", orq::aggregators::min<B>},
                               });

    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec;
    micro = end.tv_usec - begin.tv_usec;
    elapsed = seconds + micro * 1e-6;
    single_cout("MIN_AGGREGATION:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed);

    gettimeofday(&begin, 0);

    table.aggregate({"[SEL]"}, {{"[DATA]", "[MAX]", orq::aggregators::max<B>}});

    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec;
    micro = end.tv_usec - begin.tv_usec;
    elapsed = seconds + micro * 1e-6;
    single_cout("MAX_AGGREGATION:\t\t\t" << test_size << "\t\telapsed\t\t" << elapsed);

    stopwatch::done();          // print wall clock time
    stopwatch::profile_done();  // print profiling data

    runTime->print_statistics();
    runTime->print_communicator_statistics();
}

void count_rounds(const size_t test_size) {
    const std::vector<std::string> schema = {"[SEL]", "DATA", "[DATA]", "SUM", "[MAX]", "[MIN]"};
    std::vector<orq::Vector<int>> data(schema.size(), test_size);
    EncodedTable<int> table = secret_share(data, schema);

    using A = ASharedVector<int>;
    using B = BSharedVector<int>;

    print_comm_rounds("Sum agg", [&] {
        table.aggregate({"[SEL]"}, {{"DATA", "SUM", orq::aggregators::sum<A>}});
    });

    print_comm_rounds("Min agg", [&] {
        table.aggregate({"[SEL]"}, {
                                       {"[DATA]", "[MIN]", orq::aggregators::min<B>},
                                   });
    });

    print_comm_rounds("Max agg", [&] {
        table.aggregate({"[SEL]"}, {{"[DATA]", "[MAX]", orq::aggregators::max<B>}});
    });
}