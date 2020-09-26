#ifndef TINYWEBSERVER_NET_WEBSERVER_H
#define TINYWEBSERVER_NET_WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>


#include "Logger/Logger.h"
#include "Pools/ThreadPool.h"
#include "Pools/SqlConnectionPool.h"
#include "Http/HttpConn.h"
#include "Net/Epoller.h"
#include "Timer/TimerHeap.h"
#include "Config.h"

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数

class WebServer{
private:
	 //基础
    static const int MAX_FD = 65536;
    bool m_isRunning;
    int m_port;
    int m_timeoutMS;
    int m_listenFd;
    char* m_srcDir;

    uint32_t m_listenEvent;
    uint32_t m_connEvent;

    std::unique_ptr<TimerHeap> m_timer;
    std::unique_ptr<Poller> m_epoller;
    ConnectionPool* m_connPool;
    ThreadPool* m_threadPool;
    std::unordered_map<int, HttpConn> m_users;

    static int SetNoBlocking(int fd);

    bool InitSocket();
    void CloseConn(HttpConn* client);

    void DealListen();
    void AddClient(int fd, struct sockaddr_in addr);
    void SendError(int fd, const char * msg);

    void DealRead(HttpConn* client);
    void OnRead(HttpConn* client);

    void DealWrite(HttpConn* client);
    void OnWrite(HttpConn* client);

    
    void ExtentTime(HttpConn* client);
    
    

public:
	WebServer(int port, int timeoutMS, int sqlPort, 
        const std::string& sqlUser, const std::string& sqlPwd, 
        const std::string& dbName, int connPoolNum, int threadNum);
	~WebServer();
    void Start();
};

#endif