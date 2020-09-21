#ifndef TINYWEBSERVER_HTTP_HTTPREQUEST_H
#define TINYWEBSERVER_HTTP_HTTPREQUEST_H value

#include "Tools/Buffer.h"
#include <unordered_map>


class HttpRequestBase{
public:
    virtual void Init() = 0;
    virtual bool Parse(Buffer& buff) = 0;
    virtual bool IsKeepAlive() const = 0;
    virtual std::string path() const = 0;
    virtual std::string& path() = 0;
    virtual ~HttpRequestBase(){}
};

class HttpRequest final : public HttpRequestBase{
private:
    //PARSE_STATE state_;
    std::string m_method, m_path, m_version, m_body;
    std::unordered_map<std::string, std::string> m_header;
    std::unordered_map<std::string, std::string> m_post;
public:
    HttpRequest(){}
    ~HttpRequest(){}
    void Init() override{

    }
    bool Parse(Buffer& buff) override{
        return true;
    }
    bool IsKeepAlive() const override{
        return false;
    }
    
    std::string path() const override{
        return m_path;
    }
    std::string& path() override{
        return m_path;
    }
};

#endif