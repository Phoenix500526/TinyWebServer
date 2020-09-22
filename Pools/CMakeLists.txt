project(Pools CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/3rd-party/FindMySQL")    
find_package(MySQL)

set(POOLS_SRC 
	SqlConnectionPool.cc
	SqlConnectionPool.h)

add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/${POOLS_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${MYSQL_LIBRARY})

add_library(Pools::lib ALIAS ${PROJECT_NAME})