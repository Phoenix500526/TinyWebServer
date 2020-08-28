#include "LogStream.h"
#include <algorithm>
#include <assert.h>

using namespace std;

template <typename T>
size_t convert(char buf[], T val){
    static const char digits[] = "9876543210123456789";
    const char* zero = digits + 9;
    static_assert(sizeof(digits) == 20, "wrong number of digits");
    
    T tmp = val;
    char* p = buf;
    do{
        int lsd = static_cast<int>(tmp % 10);
        tmp /= 10;
        *p++ = zero[lsd];
    }while(tmp != 0);
    if(val < 0){
        *p++ = '-';
    }
    *p = '\0';
	reverse(buf, p);
    return static_cast<size_t>(p - buf);
}



template <typename T>
void LogStream::formatInteger(T val){
    if(m_buffer.remainSpace() >= kNumericSize){
        size_t len = convert(m_buffer.current(), val);
        m_buffer.add(len);
    }
}

LogStream& LogStream::operator<<(short val){
    (*this) << static_cast<int>(val);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short val){
    (*this) << static_cast<unsigned int>(val);
    return *this;
}

LogStream& LogStream::operator<<(int val){
    formatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int val){
    formatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(long val){
    formatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long val){
    formatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(long long val){
    formatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long val){
    formatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(double val){
    if(m_buffer.remainSpace() >= kNumericSize){
        int len = snprintf(m_buffer.current(), kNumericSize, "%.12g", val);
        m_buffer.add(static_cast<size_t>(len));
    }
    return *this;
}

template <typename T>
Fmt::Fmt(const char* fmt, T val){
    static_assert(std::is_arithmetic<T>::value, "Must be arithmetic type");
    m_len = snprintf(m_data, sizeof m_data, fmt, val);
    assert(static_cast<size_t>(m_len) < sizeof m_data);
}

template Fmt::Fmt(const char* fmt, char);
template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);
template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
