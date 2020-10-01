#include "WebServer.h"
#include <iostream>
using namespace std;

ConnectionPool* connPool = ConnectionPool::GetInstance();
ThreadPool* threadPool = ThreadPool::GetInstance();

WebServer::WebServer(const std::string& DBType, int port, int timeoutMS, int sqlPort, 
        const std::string& sqlUser, const std::string& sqlPwd, 
        const std::string& dbName, int connPoolNum, int threadNum)
		:m_port(port), m_timeoutMS(timeoutMS), 
		 m_timer(new TimerHeap()), m_epoller(new Epoller()),
         m_connPool(ConnectionPool::GetInstance()), m_threadPool(ThreadPool::GetInstance())
         {
			assert(port < 65535 && port > 1024);
            m_srcDir = getcwd(nullptr, 256);
			assert(m_srcDir);
            strcat(m_srcDir, "/resources");
			HttpConn::userCnt = 0;
			HttpConn::srcDir = m_srcDir;
			m_connPool->init(DBType, "localhost", sqlUser, sqlPwd, dbName, sqlPort, connPoolNum);
			m_threadPool->start(threadNum);
            m_listenEvent = (EPOLLRDHUP | EPOLLET);
            m_connEvent = (EPOLLONESHOT | EPOLLRDHUP | EPOLLET);
            HttpConn::isET = (m_connEvent & EPOLLET);
			m_isRunning = InitSocket();
			if(m_isRunning){
				LOG_INFO << "===================== Server Init Success =====================";
				LOG_INFO << "Port : " << m_port << ", srcDir :" << m_srcDir;
				LOG_INFO << "threadNum : " << threadNum << ", ConnectionNum : " << connPoolNum;
			}else{
				LOG_FATAL << "Socket Init Failure";
			}
}

WebServer::~WebServer(){
	close(m_listenFd);
	m_isRunning = false;
	free(m_srcDir);
	m_connPool->DestroyPool();
    m_threadPool->stop();
}

void WebServer::Start(){
	int timeMS = -1;
    if(m_isRunning)
        LOG_INFO << "===================== Server Start =====================";
    while(m_isRunning){
        if(m_timeoutMS > 0){
            timeMS = m_timer->GetNextTick();
        }
        int eventCnt = m_epoller->Wait(timeMS);
        for(int i = 0;i < eventCnt;++i){
            int fd = m_epoller->GetEventFd(i);
            uint32_t event = m_epoller->GetEvents(i);
            if(fd == m_listenFd){
                DealListen();
            }else{
                auto iter = m_users.find(fd);
                if(event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                    assert(iter != m_users.end());
                    CloseConn(&(iter->second));
                }
                else if(event & EPOLLIN){
                    assert(iter != m_users.end());
                    DealRead(&(iter->second));
                }
                else if(event & EPOLLOUT){
                    assert(iter != m_users.end());
                    DealWrite(&(iter->second));
                }
                else{
                    LOG_ERROR << "Unexpected Event";
                }
            }
            
        }
    }
}

void WebServer::CloseConn(HttpConn* client){
    assert(client != nullptr);
    int client_fd = client->GetFd(); 
    LOG_INFO << "Client[" << client_fd << "] quit";
    m_epoller->DelFd(client_fd);
    client->Close();
}

void WebServer::DealListen(){
    struct  sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do{
        int fd = accept(m_listenFd, (struct sockaddr*)(&addr),&len);
        if(fd <= 0){
            break;
        }
        else if(HttpConn::userCnt >= MAX_FD){
            SendError(fd, "Server Busy");
            LOG_WARN << "Client Reach Limitation";
            break;
        }
        AddClient(fd, addr);
    }while(m_listenEvent & EPOLLET);
}

void WebServer::SendError(int fd, const char* mesg){
    assert(fd > 0);
    int ret = send(fd, mesg, strlen(mesg), 0);
    if(ret < 0) {
        LOG_WARN << "send error to client[" << fd << "] error!";
    }
    close(fd);
}

void WebServer::AddClient(int fd, struct sockaddr_in addr){
    assert(fd > 0);
    m_users[fd].Init(fd, addr);
    if(m_timeoutMS > 0) {
        m_timer->AddTimer(fd, m_timeoutMS, std::bind(&WebServer::CloseConn, this, &m_users[fd]));
    }
    m_epoller->AddFd(fd, m_connEvent | EPOLLIN);
    SetNoBlocking(fd);
    LOG_INFO << "Client[" << m_users[fd].GetFd() << "] in!";
}


void WebServer::ExtentTime(HttpConn* client){
    assert(client != nullptr);
    if(m_timeoutMS > 0)
        m_timer->AdjustTimer(client->GetFd(), m_timeoutMS);
}



void WebServer::DealRead(HttpConn* client){
    assert(client != nullptr);
    ExtentTime(client);
    m_threadPool->AddTask(std::bind(&WebServer::OnRead, this, client));
}

void WebServer::OnRead(HttpConn* client){
    int ret = -1;
    int readErrno = 0;
    ret = client->Read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN){
        CloseConn(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnProcess(HttpConn* client){
    if(client->Process())
        m_epoller->ModFd(client->GetFd(), m_connEvent | EPOLLOUT);
    else
        m_epoller->ModFd(client->GetFd(), m_connEvent | EPOLLIN);
}


void WebServer::DealWrite(HttpConn* client){
    assert(client != nullptr);
    ExtentTime(client);
    m_threadPool->AddTask(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::OnWrite(HttpConn* client){
    int ret = -1;
    int writeErrno = 0;
    ret = client->Write(&writeErrno);
    if(client->ToWriteBytes() == 0){
        /* 传输完成 */
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    }else if(ret < 0){
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            m_epoller->ModFd(client->GetFd(), m_connEvent | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
}


bool WebServer::InitSocket(){
	int ret;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_port);
	struct linger optLinger = {1,1};
	m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if(m_listenFd < 0){
		LOG_ERROR << "Create Socket Failure";
		return false;
	}

	ret = setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(m_listenFd);
        LOG_ERROR << "Init Linger Error!";
        return false;
    }
    
    int optval = 1;
    /* 端口复用 */
    /* 。但是，这些套接字并不是所有都能读取信息，只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR << "Set Socket Error !";
        close(m_listenFd);
        return false;
    }

    ret = bind(m_listenFd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR << "Bind Port: "  << m_port << " Error!";
        close(m_listenFd);
        return false;
    }

    ret = listen(m_listenFd, 6);
    if(ret < 0) {
        LOG_ERROR << "Listen port: " << m_port << " Error!";
        close(m_listenFd);
        return false;
    }
    ret = m_epoller->AddFd(m_listenFd,  m_listenEvent | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR << "Add listen error!";
        close(m_listenFd);
        return false;
    }
    SetNoBlocking(m_listenFd);
    LOG_INFO << "Server port: " <<  m_port;
    return true;

}


int WebServer::SetNoBlocking(int fd){
	assert(fd > 0);
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}