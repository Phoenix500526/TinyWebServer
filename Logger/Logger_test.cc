#include "Logger.h"
#include "LogStream.h"
#include "LogFile.h"

#include <stdio.h>
using namespace std;

int TimezoneId = 8;

int main(int argc, char* argv[]){

	const char* filename = basename(argv[0]);
	LogFile log(filename, 1024);
	Logger::setOutput(bind(&LogFile::append, &log, placeholders::_1,placeholders::_2));
	{
		LOG_TRACE << "This is TRACE level";
		LOG_DEBUG << "This is DEBUG level";
		LOG_INFO << "This is INFO level";
		LOG_WARN << "This is WARN level";
		LOG_ERROR << "This is ERROR level";
		//LOG_FATAL << "This is FATAL level";
	}
	printf("没有被中断\n");
	return 0;
}
