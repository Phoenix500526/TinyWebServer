#ifndef TINYWEBSERVER_LOGGER_LOGGER_H
#define TINYWEBSERVER_LOGGER_LOGGER_H

#include "AsyncLogging.h"
#include "LogStream.h"

#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#include <functional>

class Time;
class Logger {
 public:
  typedef std::function<void(const char*, int len)> OutputFunc;
  typedef std::function<void()> FlushFunc;

  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_OF_LEVEL,
  };

  class SourceFile {
   public:
    const char* m_data;
    int m_len;

    template <unsigned N>
    SourceFile(const char (&arr)[N]) : m_data(arr), m_len(N - 1) {
      const char* slash = strrchr(arr, '/');
      if (slash) {
        m_data = slash + 1;
        m_len -= static_cast<int>(m_data - arr);
      }
    }

    explicit SourceFile(const char* file) : m_data(file) {
      const char* slash = strrchr(file, '/');
      if (slash) m_data = slash + 1;
      m_len = static_cast<int>(strlen(m_data));
    }
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool isAbort);
  ~Logger();
  LogStream& stream() { return m_impl.m_stream; }
  static LogLevel getLogLevel();
  static void setLogLevel(LogLevel level);
  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);

 private:
  class Impl {
   public:
    LogLevel m_level;
    SourceFile m_basename;
    int m_line;
    LogStream m_stream;

   public:
    void finish();
    void formatTime();
    Impl(LogLevel level, int Errno, const SourceFile& file, int line);
  };
  Impl m_impl;
};

extern Logger::LogLevel g_LogLevel;

inline Logger::LogLevel Logger::getLogLevel() { return g_LogLevel; }

const char* strerror_tl(int SavedErrno);

#define LOG_TRACE                             \
  if (Logger::getLogLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()

#define LOG_DEBUG                             \
  if (Logger::getLogLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()

#define LOG_INFO \
  if (Logger::getLogLevel() <= Logger::INFO) Logger(__FILE__, __LINE__).stream()

#define LOG_WARN                             \
  if (Logger::getLogLevel() <= Logger::WARN) \
  Logger(__FILE__, __LINE__, Logger::WARN).stream()

#define LOG_ERROR                             \
  if (Logger::getLogLevel() <= Logger::ERROR) \
  Logger(__FILE__, __LINE__, false).stream()

#define LOG_FATAL                             \
  if (Logger::getLogLevel() <= Logger::FATAL) \
  Logger(__FILE__, __LINE__, true).stream()

#define CHECKNOTNULL(val) \
  CheckNotNULL(__FILE__, __LINE__, "'" #val "' Must be not NULL", (val))

template <typename T>
T* CheckNotNULL(const Logger::SourceFile& file, int line, const char* name,
                T* ptr) {
  if (ptr == NULL) {
    Logger(file, line, Logger::FATAL).stream() << name;
  }
  return ptr;
}

#endif
