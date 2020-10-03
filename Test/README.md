## 测试目录
本目录中的测试代码主要分为两类，一类是单元测试和功能测试

## 单元测试
Buffer_unittest.cc
HttpResponse_Unittest.cc
HttpConn_Unittest.cc
Config_unittest.cc
HttpRequest_Unittest.cc
TimerHeap_Unittest.cc

单元测试可以通过执行以下命令来自动执行：`make test`，测试后信息位于 build/Testing/Temporary/LastTest.log 

## 功能测试
AsyncLogging_test.cc
CountDownLatch_test.cc
HttpResponseFile_test.html
Epoller_Echotest.cc
ThreadPool_test.cc
HttpResponseFile_test.cc

编译后的测试文件位于 build/Test 目录下

## 参考文章
1. (Googletest Primer)[https://github.com/google/googletest/blob/master/googletest/docs/primer.md]
2. (googletest测试框架初探)[https://www.jianshu.com/p/bc7a0345cb39]