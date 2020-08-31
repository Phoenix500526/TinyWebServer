project(Unit_Test)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(AsyncLogging_test AsyncLogging_test.cc)


target_link_libraries(AsyncLogging_test PRIVATE Threads::Threads Log::lib Tools::lib)


add_executable(CountDownLatch_test CountDownLatch_test.cc)

target_link_libraries(CountDownLatch_test PRIVATE Threads::Threads Tools::lib)