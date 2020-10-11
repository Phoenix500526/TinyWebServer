cmake_minimum_required(VERSION 3.5)

project(TinyWebServer)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()


set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wthread-safety )
endif()


add_compile_options(
 # -DVALGRIND
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wno-unused-parameter
 -Woverloaded-virtual
 -Wpointer-arith
 -Wwrite-strings
 )


enable_testing()

add_subdirectory(Logger)
add_subdirectory(Tools)
add_subdirectory(Pools)
add_subdirectory(Http)
add_subdirectory(Timer)
add_subdirectory(Net)
add_subdirectory(Test)	

add_executable(${PROJECT_NAME} main.cc Config.cc)
target_link_libraries(${PROJECT_NAME}
    Threads::Threads
    Log::lib
    Tools::lib
    Pools::lib
    Http::lib
    Net::lib
    Timer::lib
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR})
