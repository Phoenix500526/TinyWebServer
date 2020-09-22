#ifndef TINYWEBSERVER_HTTP_HTTPREQUEST_H
#define TINYWEBSERVER_HTTP_HTTPREQUEST_H 

#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <mysql/mysql.h>

#include "Tools/Buffer.h"
#include "Logger/Logger.h"
#include "Pools/SqlConnectionPool.h"

class HttpRequestBase{
public:
    virtual void Init() = 0;
    virtual bool Parse(Buffer& buff) = 0;
    virtual bool IsKeepAlive() const = 0;
    virtual std::string path() const = 0;
    virtual std::string& path() = 0;
    virtual std::string method() const = 0;
    virtual std::string version() const = 0;
    virtual std::string GetPost(const std::string& key) const = 0;
    virtual std::string GetPost(const char* key) const = 0;
    virtual ~HttpRequestBase(){}
};  


class HttpRequest: public HttpRequestBase{
public:
    enum class PARSE_STATE
    {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,     
    };

    enum class HTTP_CODE
    {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };    
protected:
    bool ParseRequestLine(const std::string& line);
    void ParseHeader(const std::string& line);
    void ParseBody(const std::string& line);
    void ParsePath();
    void ParsePost();
    void ParseFromUrlencoded();

    static bool UserVerify(const std::string& name, const std::string& pwd, bool Login);
    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);

    PARSE_STATE m_state;
    std::string m_method, m_path, m_version, m_body;
    std::unordered_map<std::string, std::string> m_header;
    std::unordered_map<std::string, std::string> m_post;

public:
    HttpRequest(){
        Init();
    }
    ~HttpRequest() = default;
    void Init() override;
    bool Parse(Buffer& buff) override;
    bool IsKeepAlive() const override;
    std::string path() const override;
    std::string& path() override;
    std::string method() const override;
    std::string version() const override;
    std::string GetPost(const std::string& key) const override;
    std::string GetPost(const char* key) const override;
    PARSE_STATE GetState()const{
        return m_state;
    }
};

#endif