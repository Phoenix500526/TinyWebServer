cmake_minimum_required(VERSION 3.5)

project(TinyWebServer)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(3rd-party/google-test)
enable_testing()

add_subdirectory(Logger)
add_subdirectory(Tools)
add_subdirectory(Pools)
add_subdirectory(Test)	