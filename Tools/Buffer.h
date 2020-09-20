#ifndef TINYWEBSERVER_TOOLS_BUFFER_H
#define TINYWEBSERVER_TOOLS_BUFFER_H 

#include <vector>
#include <string.h>    //perror
#include <sys/uio.h>   //readv
#include <unistd.h>
#include <assert.h>
#include <algorithm>

/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode



class Buffer
{
private:
	std::vector<char> m_buffer;
	size_t m_writerIndex;
	size_t m_readerIndex;

    void MakeSpace(size_t len);
    char* Begin(){
        return &*m_buffer.begin();
    }
    const char* Begin() const{
        return &*m_buffer.begin();
    }
    char* BeginWrite(){
        return Begin() + m_writerIndex;
    }
    const char* BeginWrite() const{
        return Begin() + m_writerIndex;
    }
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
	explicit Buffer(int initialsize = kInitialSize + kCheapPrepend)
        :m_buffer(initialsize), m_writerIndex(kCheapPrepend),m_readerIndex(kCheapPrepend){
        assert(ReadableBytes() == 0);
        assert(WritableBytes() == 1024);
        assert(PrepenableBytes() == kCheapPrepend);
    }
	~Buffer() = default;

    size_t WritableBytes() const{
        return m_buffer.size() - m_writerIndex;
    }
    size_t ReadableBytes() const{
        return m_writerIndex - m_readerIndex;
    }
    size_t PrepenableBytes() const{
        return m_readerIndex;
    }

    const char* Peek() const{
        return Begin() + m_readerIndex;
    }
    void HasWritten(size_t len){
        assert(len <= WritableBytes());
        m_writerIndex += len;
    }
    void EnsureWritable(size_t len){
        if(len > WritableBytes()){
            MakeSpace(len);
        }
        assert(WritableBytes() >= len);
    }

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);
    void RetrieveAll();
    std::string RetrieveAsString(size_t len);

    void Append(const char* data, size_t len);
    void Append(const std::string& data);
    void Append(const void* data, size_t len);


    ssize_t ReadFd(int fd, int* savedErrno);
    ssize_t WriteFd(int fd, int* savedErrno);    
	
};

#endif