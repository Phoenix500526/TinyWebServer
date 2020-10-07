#include "../Logger/AsyncLogging.h"
#include "../Logger/LogFile.h"
#include "../Logger/LogStream.h"
#include "../Logger/Logger.h"

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <chrono>
#include <iostream>
using namespace std;

off_t kRollSize = 1024 * 1000 * 1000;
AsyncLogging* g_asyncLog = nullptr;
void asyncOutput(const char* msg, int len) { g_asyncLog->append(msg, len); }

void bench(bool longLog) {
    Logger::setOutput(asyncOutput);

    int cnt = 0;
    const int kBatch = 1000;
    string empty = " ";
    string longStr(3000, 'X');
    longStr += " ";

    for (int t = 0; t < 30; ++t) {
        auto start = chrono::system_clock::now();
        for (int i = 0; i < kBatch; ++i) {
            LOG_INFO << "Hello 0123456789"
                     << " abcdefghijklmnopqrstuvwxyz "
                     << (longLog ? longStr : empty) << cnt;
            ++cnt;
        }
        auto end = chrono::system_clock::now();
        cout << chrono::duration_cast<chrono::microseconds>(end - start).count()
             << "microseconds" << endl;
        struct timespec ts = {0, 500 * 1000 * 1000};
        nanosleep(&ts, nullptr);
    }
}

int main(int argc, char* argv[]) {
    {
        // set max virtual memory to 2GB.
        size_t kOneGB = 1000 * 1024 * 1024;
        rlimit rl = {2 * kOneGB, 2 * kOneGB};
        setrlimit(RLIMIT_AS, &rl);
    }
    printf("pid = %d\n", getpid());

    char name[256] = {'\0'};
    strncpy(name, argv[0], sizeof name - 1);
    AsyncLogging log(basename(name), kRollSize);
    log.start();
    g_asyncLog = &log;
    bool longLog = argc > 1;
    bench(longLog);
}