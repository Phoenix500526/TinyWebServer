#include "Logger.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <time.h>


__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

extern int TimezoneId;

int gettid(){
    return static_cast<int>(::syscall(SYS_gettid));
}

const char* strerror_tl(int SavedErrno){
	return strerror_r(SavedErrno, t_errnobuf, sizeof t_errnobuf);
}


const char* LogLevelName[Logger::NUM_OF_LEVEL] = {
	"TRACE ",
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL "
};

Logger::LogLevel initLogLevel(){
	if(getenv("TINYWEBSERVER_LOG_TRACE")){
		return Logger::TRACE;
	}else if(getenv("TINYWEBSERVER_LOG_DEBUG")){
		return Logger::DEBUG;
	}else{
		return Logger::INFO;
	}
}

void defaultOutput(const char* msg, int len){
	int n = fwrite(msg, 1, len, stdout);
	assert(n == len);
	(void)n;
}

void defaultFlush(){
	fflush(stdout);
}

Logger::LogLevel g_LogLevel = initLogLevel();
Logger::OutputFunc g_Output = defaultOutput;
Logger::FlushFunc g_Flush = defaultFlush;

class T{
public:	
	const char* m_data;
	const unsigned m_len;
	T(const char* str, const unsigned len)
		:m_data(str), m_len(len){
			assert(m_len == strlen(m_data));
	}
};

inline LogStream& operator<<(LogStream& os, const T& val){
	os.append(val.m_data, val.m_len);
	return os;
}

inline LogStream& operator<<(LogStream& os, const Logger::SourceFile& val){
	os.append(val.m_data, val.m_len);
	return os;
}


Logger::Impl::Impl(LogLevel level, int old_Errno, const SourceFile& file, int line)
	: m_level(level),
	  m_basename(file),
	  m_line(line),
	  m_stream(){
		gettimeofday(&m_time, NULL);
		formatTime();
		char tidString[7];
		snprintf(tidString,sizeof tidString, "%5d ", gettid());
		m_stream << T(tidString, strlen(tidString));
		m_stream << T(LogLevelName[level], 6);
		if(old_Errno != 0){
			m_stream << strerror_tl(old_Errno) << " (errno = " << old_Errno << ") ";
		}
	  }

void Logger::Impl::finish(){
	m_stream << " - " << m_basename << " : " << m_line << '\n';
}

void Logger::Impl::formatTime(){
	time_t seconds = static_cast<time_t>(m_time.tv_sec);
	if(seconds != t_lastSecond){
		t_lastSecond = seconds;
		struct tm tm_time;
		gmtime_r(&seconds, &tm_time);
		int len = snprintf(t_time, sizeof t_time, "%04d%02d%02d %02d:%02d:%02d",tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour + TimezoneId, tm_time.tm_min, tm_time.tm_sec);
		assert(len == 17);
		(void)len;
	}
	Fmt us(".%06d ", m_time.tv_usec);
	assert(8 == us.length());
	m_stream << T(t_time, 17) << T(us.data(), 8);
}

Logger::Logger(SourceFile file, int line)
	:m_impl(Logger::INFO, 0, __FILE__, __LINE__){
}

Logger::Logger(SourceFile file, int line, LogLevel level)
	:m_impl(level, 0, __FILE__, __LINE__){
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
	:m_impl(level, 0, __FILE__, __LINE__){
		m_impl.m_stream << func;
}

Logger::Logger(SourceFile file, int line, bool isAbort)
	:m_impl(isAbort ? FATAL : ERROR, 0, __FILE__, __LINE__){
}


Logger::~Logger(){
	m_impl.finish();
	const LogStream::Buffer& buf = stream().buffer();
	g_Output(buf.data(), buf.length());
	if(FATAL == m_impl.m_level){
		g_Flush();
		abort();
	}
}


void Logger::setLogLevel(LogLevel level){
	g_LogLevel = level;
}

void Logger::setOutput(OutputFunc out){
	g_Output = out;
}

void Logger::setFlush(FlushFunc flush){
	g_Flush = flush;
}
