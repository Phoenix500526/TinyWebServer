#include "Net/Epoller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>         // for socket
#include <netinet/in.h>         // for sockaddr_in
#include <fcntl.h>              // for nonblocking
#include <sys/resource.h>       // for 最大连接数需要setrlimit

#define MAXEPOLL 10000         
#define MAXLINE  1024
#define PORT     6000
#define MAXBACK  1000

int setnonblocking(int fd) {
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD,0) | O_NONBLOCK) == -1) {
        printf("set blocking error: %d\n", errno);
        return -1;
    }
    return 0;
}

int main() {
    int listen_fd;
    int conn_fd;
    int nread;
    int cur_fds;     // 当前已经存在的fd数量
    int wait_fds;    
    int i;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;

    struct rlimit rlt; // 设置连接数所需
    char buf[MAXLINE];
    socklen_t len = sizeof(struct sockaddr_in);

    Epoller epoller;

    // 1.设置每个进程允许打开的最大文件数
    rlt.rlim_max = rlt.rlim_cur = MAXEPOLL;
    if(setrlimit(RLIMIT_NOFILE, &rlt) == -1) {
        printf("setrlimit error : %d\n", errno);
        exit(EXIT_FAILURE);
    }

    // 2. 设置server地址
    bzero(&servaddr, sizeof(servaddr));
    // sa_family是地址家族，一般都是“AF_xxx”的形式。通常大多用的是都是AF_INET,代表TCP/IP协议族
    servaddr.sin_family = AF_INET;
    // INADDR_ANY就是指定地址为0.0.0.0的地址,也就是表示本机的所有IP
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 必须要采用网络数据格式,普通数字可以用htons()函数转换成网络数据格式的数字
    servaddr.sin_port = htons(PORT);

    // 3. 建立套接字
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    // 4. 设置非阻塞模式
    if (setnonblocking(listen_fd) == -1) {
        printf("set non blocking error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    // 5. 绑定
    if (bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr)) == -1) {
        printf("bind error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    // 6. 监听
    if (listen(listen_fd, MAXBACK) == -1) {
        printf("listen error : %d ...\n", errno);
        exit(EXIT_FAILURE);
    }

    // 7. 创建epoll
    if (epoller.AddFd(listen_fd, EPOLLIN | EPOLLET) < 0) {
        printf("epoll_ctl error : %d\n", errno);
        exit(EXIT_FAILURE);
    }
    cur_fds = 1;

    while (true) {
        if ((wait_fds = epoller.Wait()) == -1) { 
            printf("epoll_wait error : %d\n", errno);
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < wait_fds; ++i) {
            if (epoller.GetEventFd(i) == listen_fd && cur_fds < MAXEPOLL) {
                if ((conn_fd = accept(listen_fd, (struct sockaddr *)&cliaddr, &len)) == -1) {
                    printf("accept error : %d\n", errno);
                    exit(EXIT_FAILURE);
                }
                printf("server get from client port : %d\n", cliaddr.sin_port);
                if (epoller.AddFd(conn_fd, EPOLLIN | EPOLLET) < 0) {
                    printf("epoll error : %d\n", errno);
                    exit(EXIT_FAILURE);
                }
                ++cur_fds;
                continue;
            }

            // 处理数据
            nread = read(epoller.GetEventFd(i), buf, sizeof(buf));
            if (nread < 0) {
                close(epoller.GetEventFd(i));
                epoller.DelFd(epoller.GetEventFd(i));
                --cur_fds;
                continue;
            }
            buf[nread] = 'a';
            buf[nread + 1] = '\0';
            // 回写
            write(epoller.GetEventFd(i), buf, nread);
        }
    }

    close(listen_fd);
    return 0;
}