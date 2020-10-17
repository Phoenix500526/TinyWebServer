cmake_minimum_required(VERSION 3.5)

project(TinyWebServer)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
enable_testing()

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

file(GLOB CLANG_FORMAT_BIN_NAME /usr/bin/clang-format-[3-9]*)
if(NOT CLANG_FORMAT_BIN_NAME-NOTFOUND)
    message("The clang-format is ${CLANG_FORMAT_BIN_NAME}")
    file(GLOB_RECURSE ALL_SOURCE_FILES *.cc *.h)
    add_custom_target(
            clang-format
            COMMAND ${CLANG_FORMAT_BIN_NAME}
            -style=google
            -i
            ${ALL_SOURCE_FILES}
    )
endif()


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
