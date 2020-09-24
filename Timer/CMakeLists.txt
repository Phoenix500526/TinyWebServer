project(Timer CXX)

# TimerHeap
add_library(Timer STATIC TimerHeap.cc)
target_include_directories(Timer PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Timer Log::lib)
add_library(Timer::lib ALIAS Timer)
