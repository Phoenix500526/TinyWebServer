project(Unit_Test)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

# AsyncLogging_test
add_executable(AsyncLogging_test AsyncLogging_test.cc)

target_link_libraries(AsyncLogging_test PRIVATE Threads::Threads Log::lib Tools::lib)

# CountDownLatch_test 
add_executable(CountDownLatch_test CountDownLatch_test.cc)

target_link_libraries(CountDownLatch_test PRIVATE Threads::Threads Tools::lib)

# Config_unittest
add_executable(ConfigUnitTest Config_unittest.cc ${CMAKE_SOURCE_DIR}/Config.cc)
target_include_directories(ConfigUnitTest PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(ConfigUnitTest
    GTest::GTest 
    GTest::Main
)

add_custom_command(TARGET ConfigUnitTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E 
	copy ${CMAKE_CURRENT_SOURCE_DIR}/Config_unittest.ini $<TARGET_FILE_DIR:ConfigUnitTest>)

add_test(NAME ConfigUnitTest COMMAND ConfigUnitTest)


# sqlconnectionpool_test
add_executable(SqlConnectionPool_test SqlConnectionPool_test.cc)
target_include_directories(SqlConnectionPool_test PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(SqlConnectionPool_test PRIVATE Threads::Threads Tools::lib Log::lib Pools::lib)

# threadpool_test
add_executable(ThreadPool_test ThreadPool_test.cc)
target_include_directories(ThreadPool_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(ThreadPool_test PRIVATE Threads::Threads)

# Buffer_Unittest
add_executable(BufferUnitTest Buffer_unittest.cc ${CMAKE_SOURCE_DIR}/Tools/Buffer.cc)
target_include_directories(BufferUnitTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(BufferUnitTest
    GTest::GTest
    GTest::Main
)
add_test(NAME BufferUnitTest COMMAND BufferUnitTest)

# HttpConn_Unittest
add_executable(HttpConnUnitTest HttpConn_Unittest.cc )
target_include_directories(HttpConnUnitTest PUBLIC ${CMAKE_SOURCE_DIR} ${gmock_SOURCE_DIR}/include)
target_link_libraries(HttpConnUnitTest
    GTest::GTest
    GTest::Main
    GMock::GMock
    Http::lib
)
add_test(NAME HttpConnUnitTest COMMAND HttpConnUnitTest)

#HttpRequest_Unittest
add_executable(HttpRequestUnitTest HttpRequest_Unittest.cc)
target_include_directories(HttpRequestUnitTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(HttpRequestUnitTest
    GTest::GTest
    GTest::Main
    Http::lib
)
add_test(NAME HttpRequestUnitTest COMMAND HttpRequestUnitTest)

#HttpRequestDB_test
add_executable(HttpRequestDBTest HttpRequestDB_test.cc)
target_include_directories(HttpRequestDBTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(HttpRequestDBTest
    Http::lib
    Pools::lib
)


#HttpResponse_Unittest
add_executable(HttpResponseUnitTest HttpResponse_Unittest.cc )
target_include_directories(HttpResponseUnitTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(HttpResponseUnitTest
    GTest::GTest
    GTest::Main
    Http::lib
)
add_test(NAME HttpResponseUnitTest COMMAND HttpResponseUnitTest)

#HttpResponseFile_test
add_executable(HttpResponseFileTest HttpResponseFile_test.cc )
target_include_directories(HttpResponseFileTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(HttpResponseFileTest
    Http::lib
)
add_custom_command(TARGET HttpResponseFileTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E 
    copy ${CMAKE_CURRENT_SOURCE_DIR}/HttpResponseFile_test.html $<TARGET_FILE_DIR:HttpResponseFileTest>)

#TimerHeap_Unittest
add_executable(TimerHeapUnittest TimerHeap_Unittest.cc)
target_include_directories(TimerHeapUnittest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(TimerHeapUnittest 
    GTest::GTest
    GTest::Main
    Timer::lib
)
add_test(NAME TimerHeapUnittest COMMAND TimerHeapUnittest)

#Epoller_Echotest
add_executable(EpollerEchotest Epoller_Echotest.cc)
target_include_directories(EpollerEchotest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(EpollerEchotest
    Log::lib
    Net::lib
)