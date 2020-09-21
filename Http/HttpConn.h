#ifndef TINYWEBSERVER_HTTP_HTTPCONN_H
#define TINYWEBSERVER_HTTP_HTTPCONN_H 

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      
#include <atomic>
#include <memory>

#include "Logger/Logger.h"
#include "Pools/SqlConnectionPool.h"
#include "Tools/Buffer.h"
#include "Http/HttpRequest.h"
#include "Http/HttpResponse.h"



class HttpConn{
private:
	int m_fd;
    struct  sockaddr_in m_addr;

    bool m_isClose;
    
    int m_iovCnt;
    struct iovec m_iov[2];
    
    Buffer m_readBuff; // 读缓冲区
    Buffer m_writeBuff; // 写缓冲区

    std::shared_ptr<HttpRequestBase> m_request;
    std::shared_ptr<HttpResponseBase> m_response;
public:
    static const char* srcDir;
    static std::atomic<int> userCnt;
    static bool isET;

	HttpConn();
	~HttpConn();

	void Init(int fd, const sockaddr_in& addr);
	void Reset();
	void Close();

    void SetRequest(std::shared_ptr<HttpRequestBase> request_ = std::make_shared<HttpRequest>()){
        m_request = request_;
    }
    void SetResponse(std::shared_ptr<HttpResponseBase> response_ = std::make_shared<HttpResponse>()){
        m_response = response_;
    }

	ssize_t Read(int* saveErrno);
	ssize_t Write(int* saveErrno);

	int GetFd() const;
	int GetPort() const;
	const char* GetIP() const;
	sockaddr_in GetAddr() const;

	void Process();
	int ToWriteBytes() { 
        return m_iov[0].iov_len + m_iov[1].iov_len; 
    }

    bool IsKeepAlive() const {
        return m_request->IsKeepAlive();
    }
};

#endif