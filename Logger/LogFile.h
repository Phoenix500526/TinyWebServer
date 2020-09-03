#ifndef TINYWEBSERVER_LOGGER_LOGFILE_H
#define TINYWEBSERVER_LOGGER_LOGFILE_H

#include "Tools/nocopyable.h"

#include <time.h>
#include <sys/types.h>
#include <stdio.h>
#include <string>
#include <memory>

//RAII 的方式将 fd 封装起来，这样子就不用关注 fd 的释放问题了
const std::string hostname();

class File:nocopyable{
private:
    FILE* m_fp;
    off_t m_WrittenBytes;   //记录已经写入文件的字节数
    char m_buffer[64 * 1024];
    size_t write(const char* logline, size_t len);
public:
    explicit File(const char* filename);
    explicit File(std::string const& filename);
    ~File();
    off_t getWrittenBytes(){return m_WrittenBytes;}
    void append(const char* logline, size_t len);
    void flush();
};

//LogFile 是提供给日志后端使用的，用来将缓冲区中的数据写入日志文件。
//功能：写日志文件(文件指针)、日志滚动(容量大小，滚动间隔)、记录当前写入的容量
//LogFile 不考虑线程安全问题，因为线程安全问题留给 AsyncLogging 线程去把握
class LogFile{
private:
    const std::string m_basename;
    const off_t m_RollSize;       //文件大小达到 m_RollSize 时，创建新文件
    const int m_checkEveryN;  //每插入 N 条log 就会检查当前时间是否超过上次 rollFile 的时间
    int m_count;            
    time_t m_StartOfPeriod;
    time_t m_LastRoll;
	std::unique_ptr<File> m_file;
    const static int m_kRollPerSeconds = 60*60*24;
    
public:
    LogFile(const std::string& basename, off_t m_RollSize, int checkEveryN = 1024);
    ~LogFile();
    std::string getLogFileName(const std::string& basename, time_t* now);
    void append(const char* logline, size_t len);
    bool rollFile();
    void flush();
};

#endif
