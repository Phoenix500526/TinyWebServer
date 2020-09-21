#include "Http/HttpConn.h"

using namespace std;

HttpConn::HttpConn(){
	m_fd = -1;
	m_addr = {0};
	m_isClose = true;
    SetRequest();
    SetResponse();
}

HttpConn::~HttpConn(){
	Close();
}

void HttpConn::Close(){
	m_response->UnmapFile();
	if(m_isClose == false){
		m_isClose = true;
		--userCnt;
		close(m_fd);
		LOG_INFO << "Client[" << m_fd << "](" << GetIP() << ':' << GetPort() << ") quit, UserCnt:" << static_cast<int>(userCnt);
	}
}

void HttpConn::Init(int fd, const sockaddr_in& addr){
    assert(fd > 0);
    ++userCnt;
    m_fd = fd;
    m_addr = addr;
    Reset();
    LOG_INFO << "Client[" << m_fd << "](" << GetIP() << ':' << GetPort() << ") in, UserCnt:" << static_cast<int>(userCnt);
}

const char* HttpConn::GetIP() const{
    return inet_ntoa(m_addr.sin_addr);
}

int HttpConn::GetPort() const{
	return m_addr.sin_port;
}

int HttpConn::GetFd() const{
	return m_fd;
}

struct sockaddr_in HttpConn::GetAddr() const{
	return m_addr;
};

void HttpConn::Reset(){
    m_readBuff.RetrieveAll();
    m_writeBuff.RetrieveAll();
    m_isClose = false;
    m_request->Init();
}

ssize_t HttpConn::Read(int* saveErrno){
    ssize_t len = -1;
    do{
        len = m_readBuff.ReadFd(m_fd, saveErrno);
        if(len <= 0){
            break;
        }
    }while(isET);
    return len;
}

ssize_t HttpConn::Write(int* saveErrno){
    ssize_t len = -1;
    do{
        len = writev(m_fd, m_iov, m_iovCnt);
        if(len <= 0){
            *saveErrno = errno;
            break;
        }
        if(0 == m_iov[0].iov_len + m_iov[1].iov_len){
            break;
        }else if(static_cast<size_t>(len) > m_iov[0].iov_len){
            m_iov[1].iov_base = static_cast<uint8_t*>(m_iov[1].iov_base) + (len - m_iov[0].iov_len);
            m_iov[1].iov_len -= (len - m_iov[0].iov_len);
            if(m_iov[0].iov_len){
                m_writeBuff.RetrieveAll();
                m_iov[0].iov_len = 0;
            }
        }else{
            m_iov[0].iov_base = static_cast<uint8_t*>(m_iov[0].iov_base) + len;
            m_iov[0].iov_len -= len;
            m_writeBuff.Retrieve(len);
        }
    }while(isET || ToWriteBytes() > 10240);
    return len;
}

void HttpConn::Process(){
    std::string res_path = m_request->path();
    if(m_request->Parse(m_readBuff)){
        LOG_DEBUG << "request path is " << res_path;
        m_response->Init(srcDir, res_path, m_request->IsKeepAlive(), 200);
    }else{
        m_response->Init(srcDir, res_path, false, 400);
    }
    m_response->MakeResponse(m_writeBuff);
    //响应 http 头部
    m_iov[0].iov_base = const_cast<char*>(m_writeBuff.Peek());
    m_iov[0].iov_len = m_writeBuff.ReadableBytes();
    m_iovCnt = 1;
    //响应文件
    if(m_response->FileLen() > 0 && m_response->File()){
        m_iov[1].iov_base = m_response->File();
        m_iov[1].iov_len = m_response->FileLen();
        m_iovCnt = 2;
    }
    LOG_DEBUG << "filesize: " << m_response->FileLen() << ", " << m_iovCnt << " to " << ToWriteBytes();
}