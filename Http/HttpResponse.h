#ifndef TINYWEBSERVER_HTTP_HTTPRESPONSE_H
#define TINYWEBSERVER_HTTP_HTTPRESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap
#include "Tools/Buffer.h"
#include "Logger/Logger.h"


class HttpResponseBase{
private:
    virtual void Init_Impl(const std::string& srcDir, std::string& path, bool isKeepAlive, int code) = 0;
public:
    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1){
        Init_Impl(srcDir, path, isKeepAlive, code);
    }
    virtual void UnmapFile() = 0;
    virtual void MakeResponse(Buffer& buff) = 0;
    virtual char* File() = 0;
    virtual size_t FileLen() const = 0;
    virtual void ErrorContent(Buffer&, std::string) = 0;
    virtual int Code() const = 0;
    virtual ~HttpResponseBase(){}
};



class HttpResponse:public HttpResponseBase{   
public:
    HttpResponse();
    ~HttpResponse();
    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1){
        Init_Impl(srcDir, path, isKeepAlive, code);
    }
    void UnmapFile() override;
    void MakeResponse(Buffer& buff) override;

    char* File() override;
    size_t FileLen() const override;
    void ErrorContent(Buffer&, std::string) override;
    int Code() const override;

protected:
    void Init_Impl(const std::string& srcDir, std::string& path, bool isKeepAlive, int code) override;   

    void AddStateLine(Buffer &buff);
    void AddHeader(Buffer &buff);
    void AddContent(Buffer &buff);

    void ErrorHtml();

    const std::string checkStatus();
    std::string GetFileType();

    int m_code;
    bool m_isKeepAlive;

    std::string m_path;
    std::string m_srcDir;
    
    char* m_mmFile; 
    struct stat m_mmFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> ERROR_PAGE;
};
#endif