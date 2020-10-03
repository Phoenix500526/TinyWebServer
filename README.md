# TinyWebServer
基于 C++11 实现的高性能 Web 服务器

## 实现
* 配置模块: 利用模板实现配置单元，从配置文件 config.ini 中读取配置信息并初始化服务器，支持注释语法, 文件名为Config.{h,cc}
* Logger: 日志模块借鉴了 muduo 网络库的高性能异步日志模块，支持文件滚动
* HTTP：利用有限状态机及正则表达式，实现对 HTTP 的请求报文的解析及响应报文的生成
* Timer：利用最小堆实现定时器，关闭超时的非活动连接
* Pools: 包含了连接池和线程池以及数据库访问对象 DAO
* Tools: 包含了 Buffer，CountDownLatch，Mutex，Condition 等同步工具
* Net: 包含 WebServer 和 Poller 
整个 TinyWebServer 利用 IO 复用技术及线程池实现了高并发的 Reactor 模型，采用 cmake 进行构建，开发的过程当中采用了良好的 OO 设计，符合 TDD 流程，使用 googletest 测试框架进行了单元测试，Mock 测试以及对应模块的功能测试。最终程序经 webbench 测试可实现上万并发

## 环境要求
* Ubuntu 16.04.2 LTS
* C++ 11
* MySql 
* clang version 3.8.0-2ubuntu4 或 gcc version 5.4.0 20160609 


## 源码树及描述信息
```shell
.
├── 3rd-party
│   ├── FindMySQL
│   │   └── FindMySQL.cmake
│   └── google-test
│       ├── CMakeLists.txt
│       └── CMakeLists.txt.in
├── build 
├── CMakeLists.txt
├── Config.cc
├── Config.h
├── config.ini
├── Http
│   ├── README.md
│   ├── CMakeLists.txt
│   ├── HttpConn.cc
│   ├── HttpConn.h
│   ├── HttpRequest.cc
│   ├── HttpRequest.h
│   ├── HttpResponse.cc
│   └── HttpResponse.h
├── Logger
│   ├── README.md
│   ├── AsyncLogging.cc
│   ├── AsyncLogging.h
│   ├── CMakeLists.txt
│   ├── LogFile.cc
│   ├── LogFile.h
│   ├── Logger.cc
│   ├── Logger.h
│   ├── LogStream.cc
│   └── LogStream.h
├── main.cc
├── Net
│   ├── README.md
│   ├── CMakeLists.txt
│   ├── Epoller.cc
│   ├── Epoller.h
│   ├── WebServer.cc
│   └── WebServer.h
├── Pools
│   ├── README.md
│   ├── CMakeLists.txt
│   ├── DAO.h
│   ├── SqlConnectionPool.cc
│   ├── SqlConnectionPool.h
│   └── ThreadPool.h
├── resources
├── Test
│   ├── README.md
│   ├── AsyncLogging_test.cc
│   ├── Buffer_unittest.cc
│   ├── CMakeLists.txt
│   ├── Config_unittest.cc
│   ├── Config_unittest.ini
│   ├── CountDownLatch_test.cc
│   ├── Epoller_Echotest.cc
│   ├── HttpConn_Unittest.cc
│   ├── HttpRequest_Unittest.cc
│   ├── HttpResponseFile_test.cc
│   ├── HttpResponseFile_test.html
│   ├── HttpResponse_Unittest.cc
│   ├── ThreadPool_test.cc
│   └── TimerHeap_Unittest.cc
├── Timer
│   ├── README.md
│   ├── CMakeLists.txt
│   ├── TimerHeap.cc
│   └── TimerHeap.h
└── Tools
    ├── README.md
    ├── Buffer.cc
    ├── Buffer.h
    ├── CMakeLists.txt
    ├── Condition.h
    ├── CountDownLatch.cc
    ├── CountDownLatch.h
    ├── Mutex.h
    └── nocopyable.h
```

## 编译方法
```shell
$ mkdir build
$ cd build/
$ cmake .. -DCMAKE_CXX_COMPILER=clang++3.8
# 可先对代码进行单元测试
$ make test
$ make install -j4
```

## 环境配置
```shell
# 建立yourdb库
$ create database yourdb;

// 在数据库中建立用户表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, passwd) VALUES('name', 'password');
```

## 致谢
Linux高性能服务器编程，游双著.