## Net 目录
该目录下主要包含两个类：WebServer 和 Epoller 的实现。

## WebServer
WebServer 对 Linux 网络编程的各个 API 进行了相应的封装，结合了 IO 复用技术及 Reactor 模型(即 one event loop per thread)实现了高并发。

## Epoller
Epoller 则对 epoll 的相关 API 进行了封装。