#ifndef TINYWEBSERVER_TOOLS_BUFFER_H
#define TINYWEBSERVER_TOOLS_BUFFER_H

#include <assert.h>
#include <string.h>   //perror
#include <sys/uio.h>  //readv
#include <unistd.h>
#include <algorithm>
#include <vector>

/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
private:
    std::vector<char> m_buffer;
    size_t m_writerIndex;
    size_t m_readerIndex;

    void MakeSpace(size_t len);
    char* Begin() { return &*m_buffer.begin(); }
    const char* Begin() const { return &*m_buffer.begin(); }

public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    //仅供 Mock 测试匹配参数使用，只声明不定义
    friend bool operator==(const Buffer& lhs, const Buffer& rhs);
    explicit Buffer(int initialsize = kInitialSize + kCheapPrepend)
        : m_buffer(initialsize),
          m_writerIndex(kCheapPrepend),
          m_readerIndex(kCheapPrepend) {
        assert(ReadableBytes() == 0);
        assert(WritableBytes() == 1024);
        assert(PrepenableBytes() == kCheapPrepend);
    }
    ~Buffer() = default;

    char* BeginWrite() { return Begin() + m_writerIndex; }
    const char* BeginWrite() const { return Begin() + m_writerIndex; }

    size_t WritableBytes() const { return m_buffer.size() - m_writerIndex; }
    size_t ReadableBytes() const { return m_writerIndex - m_readerIndex; }
    size_t PrepenableBytes() const { return m_readerIndex; }

    const char* Peek() const { return Begin() + m_readerIndex; }
    void HasWritten(size_t len) {
        assert(len <= WritableBytes());
        m_writerIndex += len;
    }
    void EnsureWritable(size_t len) {
        if (len > WritableBytes()) {
            MakeSpace(len);
        }
        assert(WritableBytes() >= len);
    }

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);
    void RetrieveAll();
    std::string RetrieveAsString(size_t len);
    std::string RetrieveAllAsString();

    void Append(const char* data, size_t len);
    void Append(const std::string& data);
    void Append(const void* data, size_t len);

    ssize_t ReadFd(int fd, int* savedErrno);
    ssize_t WriteFd(int fd, int* savedErrno);
};

#endif