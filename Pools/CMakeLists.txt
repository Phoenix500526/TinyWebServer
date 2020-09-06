project(Pools CXX)

set(POOLS_SRC 
	SqlConnectionPool.cc
	SqlConnectionPool.h)

add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/${POOLS_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

add_library(Pools::lib ALIAS ${PROJECT_NAME})