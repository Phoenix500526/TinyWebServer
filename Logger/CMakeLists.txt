project(Logger)

set(LOGGER_SRC
	AsyncLogging.cc
	AsyncLogging.h
	LogFile.cc
	LogFile.h
	Logger.cc
	Logger.h
	LogStream.cc
	LogStream.h
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCE_DIR}/${LOGGER_SRC})
add_library(Log::lib ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})