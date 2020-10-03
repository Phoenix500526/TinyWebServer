## Tools 目录
该目录中存放了 TinyWebServer 所使用到的一些工具类，包括 Buffer、CountDownLatch、Mutex 和 Condition 等。其中 Mutex 和 Condition 对 C++ 标准库的 mutex.h 和 conditional_variable.h 进行了一层薄薄的封装，其目的是为了使用 clang 的线程安全注解功能(TSA)，该功能能够在编译期间检测到一些可能导致死锁，竞态条件的问题。启动 TSA 的编译选项为 -Wthread-safety

## 参考文章
1. [Clang 12 documentation - Thread Safty Analysis](https://clang.llvm.org/docs/ThreadSafetyAnalysis.html#reference-guide) 
2. [clang 的线程安全分析笔记](https://www.jianshu.com/p/e6fd088f96c2)