cmake_minimum_required(VERSION 3.5)

project(TinyWebServer)

set(CMAKE_CXX_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(3rd-party/google-test)
enable_testing()

add_subdirectory(Logger)
add_subdirectory(Tools)
add_subdirectory(Pools)
add_subdirectory(Http)
add_subdirectory(Timer)
add_subdirectory(Net)
add_subdirectory(Test)	