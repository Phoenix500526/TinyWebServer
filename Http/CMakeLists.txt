Project(Http CXX)

set(HTTP_SRC
    HttpConn.cc
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCE_DIR}/${HTTP_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

add_library(Http::lib ALIAS ${PROJECT_NAME})
