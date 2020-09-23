#include "Buffer.h"

using namespace std;

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;


void Buffer::MakeSpace(size_t len){
    if(WritableBytes() + PrepenableBytes() < len + kCheapPrepend){
        m_buffer.resize(len + m_writerIndex);
    }else{
        assert(kCheapPrepend < m_readerIndex);
        size_t readable = ReadableBytes();
        std::copy(Begin() + m_readerIndex, Begin() + m_writerIndex, Begin() + kCheapPrepend );
        m_readerIndex = kCheapPrepend;
        m_writerIndex = m_readerIndex + readable;
        assert(ReadableBytes() == readable);
    }
}

void Buffer::Retrieve(size_t len){
    assert(ReadableBytes() >= len);
    if(ReadableBytes() > len){
        m_readerIndex += len;
    }else{
        RetrieveAll();
    }
}


void Buffer::RetrieveUntil(const char* end){
    assert(Peek() <= end);
    assert(end <= BeginWrite());
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll(){
    m_writerIndex = kCheapPrepend;
    m_readerIndex = kCheapPrepend;
}

string Buffer::RetrieveAsString(size_t len){
    assert(len <= ReadableBytes());
    string result(Peek(), len);
    Retrieve(len);
    return result;
}

string Buffer::RetrieveAllAsString(){
    return RetrieveAsString(ReadableBytes());
}

void Buffer::Append(const char* data, size_t len){
    EnsureWritable(len);
    copy(data, data + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const std::string& data){
    Append(data.c_str(), data.size());
}

void Buffer::Append(const void* data, size_t len){
    Append(static_cast<const char*>(data), len);
}

// scatter read
ssize_t Buffer::ReadFd(int fd, int* savedErrno){
    char buf[65535];
    struct iovec vec[2];
    const size_t writable = WritableBytes();
    vec[0].iov_base = Begin() + m_writerIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = buf;
    vec[1].iov_len = sizeof buf;
    const ssize_t len = readv(fd, vec, 2);
    if(len < 0){
        *savedErrno = errno;
    }else if(static_cast<size_t>(len) < writable){
        m_writerIndex += len;
    }else{
        m_writerIndex = m_buffer.size();
        Append(buf, len - writable);
    }
    return len;
}

//gather write
ssize_t Buffer::WriteFd(int fd, int* savedErrno){
    const size_t readable = ReadableBytes();
    ssize_t len = write(fd, Peek(), readable);
    if(len < 0){
        *savedErrno = errno;
    }else{
        m_readerIndex += len;
    }
    return len;
}