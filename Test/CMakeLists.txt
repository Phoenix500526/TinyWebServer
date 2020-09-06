project(Unit_Test)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

# AsyncLogging_test
add_executable(AsyncLogging_test AsyncLogging_test.cc)

target_link_libraries(AsyncLogging_test PRIVATE Threads::Threads Log::lib Tools::lib)

# CountDownLatch_test 
add_executable(CountDownLatch_test CountDownLatch_test.cc)

target_link_libraries(CountDownLatch_test PRIVATE Threads::Threads Tools::lib)

# Config_unitest
add_executable(ConfigUnitTest Config_unittest.cc ${CMAKE_SOURCE_DIR}/Config.cc)
target_include_directories(ConfigUnitTest PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(ConfigUnitTest
    GTest::GTest 
    GTest::Main
)

add_custom_command(TARGET ConfigUnitTest POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E 
	copy ${CMAKE_CURRENT_SOURCE_DIR}/Config_unittest.ini $<TARGET_FILE_DIR:ConfigUnitTest>)

add_test(test_all ConfigUnitTest)

# sqlconnectionpool_test
add_executable(SqlConnectionPool_test SqlConnectionPool_test.cc)
target_include_directories(SqlConnectionPool_test PUBLIC ${CMAKE_SOURCE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/3rd-party/FindMySQL")    
find_package(MySQL)

target_link_libraries(SqlConnectionPool_test PRIVATE Threads::Threads Tools::lib ${MYSQL_LIBRARY} Log::lib Pools::lib)
