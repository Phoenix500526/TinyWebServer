project(Tools)

set(TOOLS_SRC
	CountDownLatch.cc
    Buffer.cc)

add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/${TOOLS_SRC})
add_library(Tools::lib ALIAS ${PROJECT_NAME})
