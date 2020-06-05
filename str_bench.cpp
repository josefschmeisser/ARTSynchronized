#include <iostream>
#include <chrono>
#include "tbb/tbb.h"

using namespace std;

#include "OptimisticLockCoupling/Tree.h"
#include "ROWEX/Tree.h"
#include "ART/Tree.h"

#include "third_party/profile.hpp"
#include "zipf.hpp"

/*
void loadKey(TID tid, Key &key) {
    // Store the key of the tuple into the key vector
    // Implementation is database specific
    key.setKeyLen(sizeof(tid));
    reinterpret_cast<uint64_t *>(&key[0])[0] = __builtin_bswap64(tid);
}
*/

static vector<char*> keys;

void loadKey(TID tid, Key &key) {
//    printf("load %lu\n", tid); cout << endl;
    key.set(keys[tid], strlen(keys[tid])); // TODO store length
}

std::string getFileName(const std::string& s) {
    char sep = '/';

#ifdef _WIN32
    sep = '\\';
#endif

    size_t i = s.rfind(sep, s.length());
    if (i != std::string::npos) {
        return(s.substr(i+1, s.length() - i));
    }

    return("");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << argv[0] << " <filename>" << std::endl;
        return 0;
    }

    ART_OLC::Tree tree(loadKey);
    //ART_ROWEX::Tree tree(loadKey);

    char* buffvar = nullptr;
    size_t buffsize = 0;
    FILE* file = fopen(argv[1], "r");
    auto threadInfo = tree.getThreadInfo();
    cout << "inserting data..." << endl;
    while (getline(&buffvar, &buffsize, file) != -1) {
        unsigned len = strnlen(buffvar, buffsize);
        buffvar[len - 1] = 'x';
        keys.push_back(buffvar);
        buffvar = nullptr;
    }

    uint64_t n = keys.size();
    for (uint64_t tid = 0; tid < n; ++tid) {
        Key key;
        loadKey(tid, key);
        tree.insert(key, tid, threadInfo);
    }

    const std::string approach = "ART";

    size_t repetitions = 1;
    char* repetitionsEnv = getenv("REPETITIONS");
    if (repetitionsEnv) {
        repetitions = std::atoi(repetitionsEnv);
    }

    uint32_t threads = std::thread::hardware_concurrency();
    char* threadsEnv = getenv("THREADS");
    if (threadsEnv) {
        threads = std::atoi(threadsEnv);
    }
    tbb::task_scheduler_init taskScheduler(threads); // FIXME this feature is deprecated

    string dataset("'");
    dataset += getFileName(argv[1]);
    dataset += "'";

    PerfEvents e;
    {
        e.timeAndProfile("lookup", n,
            [&]() {
                auto localThreadInfo = tree.getThreadInfo();
                for (uint64_t i = 0; i < n; i++) {
                    Key key;
                    loadKey(i, key);
                    auto ret = tree.lookup(key, localThreadInfo); // FIXME
                    (void)ret;
  //                  if (ret != i) { throw; }
    //                assert(ret == i);
                }
            },
            repetitions, {{"approach", approach}, {"dataset", dataset}, {"threads", std::to_string(1)}});
    }

    {
        std::vector<uint64_t> skewed(n);
        std::mt19937 generator;
        generator.seed(0);
        zipf_distribution<uint64_t> zipfDistribution(n, 1.25);
        for (uint64_t i = 0; i < n; ++i) {
            skewed[i] = zipfDistribution(generator);
        }

        e.timeAndProfile("lookupSkewed", n,
            [&]() {
                auto localThreadInfo = tree.getThreadInfo();
                for (uint64_t i = 0; i < n; i++) {
                    Key key;
                    const auto j = skewed[i];
                    loadKey(j, key);
                    auto ret = tree.lookup(key, localThreadInfo); // FIXME
                    (void)ret;
  //                  if (ret != j) { throw; }
    //                assert(ret == j);
                }
            },
            repetitions, {{"approach", approach}, {"dataset", dataset}, {"threads", std::to_string(1)}});
    }

    {
        e.timeAndProfile("lookupParallel", n,
            [&]() {
                tbb::parallel_for(tbb::blocked_range<uint32_t>(0, n), [&](const tbb::blocked_range<uint32_t>& range) {
                    auto localThreadInfo = tree.getThreadInfo();
                    for (uint32_t i = range.begin(); i < range.end(); i++) {
                        Key key;
                        loadKey(i, key);
                        auto ret = tree.lookup(key, localThreadInfo); // FIXME
                        (void)ret;
                    }
                });
            },
            repetitions, {{"approach", approach}, {"dataset", dataset}, {"threads", std::to_string(threads)}});
    }

    return 0;
}
