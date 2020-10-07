#ifndef TINYWEBSERVER_LOGGER_LOGSTREAM_H
#define TINYWEBSERVER_LOGGER_LOGSTREAM_H

#include "Tools/nocopyable.h"

#include <string.h>
#include <string>

/* LogStream 的作用是作为日志的输入流来使用。
 * 功能：
 * 1、针对各种类型的 << 操作符；提供一个能高效将字符串转换为整数的函数
 * 2、格式控制 (Fmt 对象来实现)
 * 3、固定大小的缓冲区 (FixedBuffer 来实现)
 * 【追加、复位、清空、判断位置指针是否抵达末尾、获得当前缓冲区内数据长度、剩余空间、返回当前位置指针】
 */

//分为大小缓冲区，小缓冲区的大小大约为 4 KB，而大缓冲区的大小为 4 MB
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

// noncopable
// 是一个不可拷贝的类，其中构造和析构函数是受保护类型，而拷贝构造与拷贝赋值是删除的
template <size_t N>
class FixedBuffer : nocopyable {
private:
    char m_data[N];
    char* m_current;
    const char* end() const { return m_data + sizeof m_data; }

public:
    FixedBuffer() : m_current(m_data) {}
    const char* data() const { return m_data; }
    int length() const { return static_cast<int>(m_current - m_data); }
    char* current() const { return m_current; }
    int remainSpace() { return static_cast<int>(end() - m_current); }
    void reset() { m_current = m_data; }
    void bzero() { memset(m_data, 0, sizeof m_data); }
    void add(size_t len) { m_current += len; }
    void append(const char* data, size_t len) {
        if (static_cast<size_t>(remainSpace()) >= len) {
            memcpy(m_current, data, len);
            m_current += len;
        }
    }
    std::string toString() const { return std::string(m_data, length()); }
};

class LogStream : nocopyable {
public:
    typedef FixedBuffer<kSmallBuffer> Buffer;

private:
    Buffer m_buffer;
    static const int kNumericSize = 32;
    template <typename T>
    void formatInteger(T val);

public:
    void append(const char* buf, size_t len) { m_buffer.append(buf, len); }
    void reset() { m_buffer.reset(); }
    const Buffer& buffer() const { return m_buffer; }
    LogStream& operator<<(bool val) {
        m_buffer.append(val ? "true  " : "false ", 6);
        return *this;
    }
    LogStream& operator<<(char val) {
        m_buffer.append(&val, 1);
        return *this;
    }

    LogStream& operator<<(short val);
    LogStream& operator<<(unsigned short val);
    LogStream& operator<<(int val);
    LogStream& operator<<(unsigned int val);
    LogStream& operator<<(long val);
    LogStream& operator<<(unsigned long val);
    LogStream& operator<<(long long val);
    LogStream& operator<<(unsigned long long val);

    LogStream& operator<<(float val) {
        (*this) << static_cast<double>(val);
        return *this;
    }
    LogStream& operator<<(double val);

    LogStream& operator<<(char* p_str) {
        if (p_str) {
            m_buffer.append(p_str, strlen(p_str));
        } else {
            m_buffer.append("(nullptr)", 9);
        }
        return *this;
    }
    LogStream& operator<<(const char* p_str) {
        return operator<<(const_cast<char*>(p_str));
    }
    LogStream& operator<<(const std::string& str) {
        return operator<<(const_cast<char*>(str.c_str()));
    }
    LogStream& operator<<(std::string& str) {
        m_buffer.append(str.c_str(), str.size());
        return *this;
    }
};

class Fmt {
private:
    char m_data[32];
    int m_len;

public:
    template <typename T>
    Fmt(const char* fmt, T val);
    const char* data() const { return m_data; }
    int length() const { return m_len; }
};

inline LogStream& operator<<(LogStream& ls, Fmt fmt) {
    ls.append(fmt.data(), fmt.length());
    return ls;
}

#endif
