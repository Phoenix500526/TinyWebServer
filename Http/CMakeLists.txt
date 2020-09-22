Project(Http CXX)

set(HTTP_SRC
    HttpConn.cc
    HttpRequest.cc
    HttpResponse.cc
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCE_DIR}/${HTTP_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Pools::lib Log::lib Tools::lib)

add_library(Http::lib ALIAS ${PROJECT_NAME})
