#ifndef TINYWEBSERVER_HTTP_HTTPRESPONSE_H
#define TINYWEBSERVER_HTTP_HTTPRESPONSE_H value

#include <iostream>

class Buffer;

class HttpResponseBase{
public:
    virtual void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1) = 0;
    virtual void UnmapFile() = 0;
    virtual void MakeResponse(Buffer& buff) = 0;
    virtual char* File() = 0;
    virtual size_t FileLen() const = 0;
    virtual ~HttpResponseBase(){}
};



class HttpResponse final:public HttpResponseBase{
public:
    HttpResponse(){}
    ~HttpResponse(){}
    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1) override{
        std::cout << "Init" << std::endl;
    }

    void UnmapFile() override{

    }
    void MakeResponse(Buffer& buff) override{

    }
    char* File() override{
        return nullptr;
    }  
    size_t FileLen() const override{
        return 10;
    }
};
#endif