## Http 模块
Http 模块一共包含了 HttpConn、HttpRequest 以及 HttpResponse 三个类，其中 HttpConn 占据核心地位，并向上层用户提供 Process 接口。Process 接口会调用 HttpRequest 的 Parse 接口解析请求报文，并根据解析结果初始化 HttpResponse 对象，然后响应请求。为了提高文件写入缓冲区的效率，采用了分散读，集中写的方式。

## 测试
HttpConn_Unittest.cc 文件为 HttpConn 的单元测试文件，并对 HttpRequest 和 HttpResponse 接口进行了 mock 测试。
HttpRequest_Unittest.cc 文件为 HttpRequest 的单元测试文件
HttpResponse_Unittest.cc 文件为 HttpResponse 的单元测试文件
HttpResponseFile_test.cc 文件为 HttpResponse 的文件读写功能测试
上述测试文件均位于 Test/ 目录中, 编译后的测试文件位于 build/Test 目录中，功能测试需要手动执行，单元测试可通过 `make test` 命令自动执行，执行后的测试信息位于 build/Testing/Temporary/LastTest.log 当中