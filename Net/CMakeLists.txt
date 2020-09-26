project(Net CXX)

#EPoller
set(NET_SRC_CODE
    Epoller.cc
    WebServer.cc
)

add_library(${PROJECT_NAME} STATIC ${NET_SRC_CODE})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Log::lib
    Http::lib
    Pools::lib
)

add_library(Net::lib ALIAS ${PROJECT_NAME})