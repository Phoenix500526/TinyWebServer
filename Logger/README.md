## 高性能异步日志模块
TinyWebServer 的异步日志模块参考了 muduo 网络库的高性能日志设计，本质上是一个多生产者单消费者的模型，分为前端(frontend)和后端(backend).
* 前端：主要供用户使用，提供了 TRACE,DEBUG,INFO,WARN,ERROR,FATAL 等 6 个日志输出级别，默认级别为 INFO。前端的日志打印采用了流式风格，出于安全性及性能的考虑，封装了一个简单的日志流 LogStream，其优点是易用性更强，但相应地也带来了两个问题：
    * 实现格式化输出比较麻烦 [通过增加一层间接性，实现一个 Fmt 的格式类，并重载相应的流操作符即可解决]
    * 线程安全性问题        [日志前端供多个用户使用，如果为其上锁，则极端情况下会变成串行模式，难免影响调用者自身的运行。线程安全性可以通过栈上空间及匿名对象来解决。]
* 后端：后端分为 AsyncLogging 和 LogFile 两个模块，其中 AsyncLogging 和前端的 Logger 打交道，属于多对一的模式，需要用锁来避免竞态的出现。后端使用了双缓冲技术，为了避免短时间内爆炸性地增加许多 log，AsyncLogging 中还留有 2 个预备缓冲。当出现某种情况，如程序陷入了死循环，使得日志信息超过了 AsyncLogging 的处理上限(25 个缓冲区)，AsyncLogging 丢弃多余的日志信息，并向用户发送错误信息。

## 相关测试
Logger 的测试源码文件位于 Test/AsyncLogging_test.cc。分别对短日志以及长日志的打印功能及性能进行了相应的测试。编译后的可执行文件路径为 build/Test/AsyncLogging_test 

## 相关文章
关于 muduo 网络库的日志模块分析可以参见我的博客：
* [muduo 网络库日志前端](https://www.jianshu.com/p/3fe502ccfa8f)
* [muduo 网络库日志系统后端](https://www.jianshu.com/p/6cec2ff8157b)
