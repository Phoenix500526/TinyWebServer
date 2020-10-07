#include "Http/HttpRequest.h"
#include <gtest/gtest.h>
#include "Tools/Buffer.h"

class HttpRequest_Derived : public HttpRequest {
public:
    bool ParseRequestLine(const std::string& line) {
        return HttpRequest::ParseRequestLine(line);
    }

    void ParsePath() { HttpRequest::ParsePath(); }

    void ParseHeader(const std::string& line) {
        HttpRequest::ParseHeader(line);
    }

    void ParseFromUrlencoded(const std::string& line) {
        HttpRequest::m_body = line;
        HttpRequest::ParseFromUrlencoded();
    }

    bool Parse(Buffer& buf) { return HttpRequest::Parse(buf); }

    std::string GetHeaderSecond(const std::string& key) {
        if (GetHeaderFirst(key)) return HttpRequest::m_header[key];
        return "";
    }

    bool GetHeaderFirst(const std::string& key) {
        if (HttpRequest::m_header.find(key) != HttpRequest::m_header.end())
            return true;
        else
            return false;
    }

    std::string GetPostSecond(const std::string& key) {
        if (GetPostFirst(key)) return HttpRequest::m_post[key];
        return "";
    }

    bool GetPostFirst(const std::string& key) {
        if (HttpRequest::m_post.find(key) != HttpRequest::m_post.end())
            return true;
        else
            return false;
    }

    std::string& path() { return HttpRequest::path(); }

    std::string path() const { return HttpRequest::path(); }

    std::string method() const { return HttpRequest::method(); }

    std::string version() const { return HttpRequest::version(); }

    HttpRequest::PARSE_STATE GetState() { return HttpRequest::GetState(); }

    int ConverHex(char c) { return HttpRequest::ConverHex(c); }

    bool IsKeepAlive() const { return HttpRequest::IsKeepAlive(); }

    static void AddUser(const std::string& name, const std::string& passwd) {
        HttpRequest::USERTABLE[name] = passwd;
    }

    HttpRequest::LOGIN_STATUS Login(const std::string& name,
                                    const std::string& passwd) {
        return HttpRequest::Login(name, passwd);
    }
};

TEST(HttpRequest_Unittest, ParseRequestLineAndPathTest) {
    HttpRequest_Derived request_1;
    EXPECT_EQ(request_1.GetState(), HttpRequest::PARSE_STATE::REQUEST_LINE);
    std::string emptyReqLine;
    EXPECT_EQ(request_1.ParseRequestLine(emptyReqLine), false);
    EXPECT_EQ(request_1.GetState(), HttpRequest::PARSE_STATE::REQUEST_LINE);

    HttpRequest_Derived request_2;
    EXPECT_EQ(request_2.GetState(), HttpRequest::PARSE_STATE::REQUEST_LINE);
    std::string wrongReqLine("Hello World!");
    EXPECT_EQ(request_2.ParseRequestLine(wrongReqLine), false);
    EXPECT_EQ(request_2.GetState(), HttpRequest::PARSE_STATE::REQUEST_LINE);

    HttpRequest_Derived request_3;
    std::string correctReqLine("POST / HTTP/1.1");

    std::string method("POST"), path("/"), version("1.1");
    EXPECT_EQ(request_3.GetState(), HttpRequest::PARSE_STATE::REQUEST_LINE);
    EXPECT_EQ(request_3.ParseRequestLine(correctReqLine), true);
    EXPECT_EQ(request_3.path(), path);
    request_3.ParsePath();
    EXPECT_EQ(request_3.path(), std::string("/index.html"));
    EXPECT_EQ(request_3.method(), method);
    EXPECT_EQ(request_3.version(), version);
    EXPECT_EQ(request_3.GetState(), HttpRequest::PARSE_STATE::HEADERS);

    HttpRequest_Derived request_4;
    std::string loginReqLine("POST /login HTTP/1.1");
    EXPECT_EQ(request_4.ParseRequestLine(loginReqLine), true);
    request_4.ParsePath();
    EXPECT_EQ(request_4.path(), std::string("/login.html"));

    HttpRequest_Derived request_5;
    std::string otherReqLine("POST /form/entry HTTP/1.1");
    EXPECT_EQ(request_5.ParseRequestLine(otherReqLine), true);
    request_5.ParsePath();
    EXPECT_EQ(request_5.path(), std::string("/form/entry"));
}

TEST(HttpRequest_Unittest, ParseHeaderTest) {
    HttpRequest_Derived request;
    std::string line("Host: google.hk");
    request.ParseHeader(line);
    EXPECT_EQ(request.GetHeaderFirst("Host"), true);
    EXPECT_EQ(request.GetHeaderSecond("Host"), std::string("google.hk"));
}

TEST(HttpRequest_Unittest, ParseURLEncodedTest) {
    HttpRequest_Derived request;
    std::string line(
        "test=title&username=root&passwd=qq+555555&permission=normal");

    EXPECT_EQ(request.ConverHex('A'), 10);
    EXPECT_EQ(request.ConverHex('C'), 12);
    EXPECT_EQ(request.ConverHex('F'), 15);
    EXPECT_EQ(request.ConverHex('a'), 10);
    EXPECT_EQ(request.ConverHex('c'), 12);
    EXPECT_EQ(request.ConverHex('f'), 15);

    request.ParseFromUrlencoded(line);
    EXPECT_EQ(request.GetPostFirst("test"), true);
    EXPECT_EQ(request.GetPostSecond("test"), std::string("title"));

    EXPECT_EQ(request.GetPostFirst("username"), true);
    EXPECT_EQ(request.GetPostSecond("username"), std::string("root"));

    EXPECT_EQ(request.GetPostFirst("passwd"), true);
    EXPECT_EQ(request.GetPostSecond("passwd"), std::string("qq 555555"));

    EXPECT_EQ(request.GetPostFirst("permission"), true);
    EXPECT_EQ(request.GetPostSecond("permission"), std::string("normal"));
}

TEST(HttpRequest_Unittest, ParseFiledTest) {
    HttpRequest_Derived request;
    Buffer testBuf;
    EXPECT_EQ(request.Parse(testBuf), false);

    std::string wrongReqLine("Hello World!!\r\n");
    testBuf.Append(wrongReqLine);
    EXPECT_EQ(request.Parse(testBuf), false);
}

TEST(HttpRequest_Unittest, ParseTest) {
    HttpRequest_Derived request_1;
    Buffer testBuf;
    std::string PackageReqLine("POST / HTTP/1.1\r\n");
    testBuf.Append(PackageReqLine);
    EXPECT_EQ(request_1.Parse(testBuf), true);
    EXPECT_EQ(request_1.GetState(), HttpRequest::PARSE_STATE::HEADERS);

    HttpRequest_Derived request_2;
    Buffer testBuf_2;
    std::string PackageHeader("Host: google.hk\r\nConnection: keep-alive\r\n");
    testBuf_2.Append(PackageReqLine);
    testBuf_2.Append(PackageHeader);
    EXPECT_EQ(request_2.Parse(testBuf_2), true);
    EXPECT_EQ(request_2.IsKeepAlive(), true);
    EXPECT_EQ(request_2.GetHeaderFirst("Host"), true);
    EXPECT_EQ(request_2.GetHeaderSecond("Host"), std::string("google.hk"));
}

TEST(HttpRequest_Unittest, LoginTest) {
    std::string root("root");
    std::string emptyPasswd;
    std::string emptyUser;
    std::string rootPasswd("123123");
    std::string wrongUser("roo");
    std::string wrongPasswd("123");

    HttpRequest_Derived::AddUser(root, rootPasswd);
    HttpRequest_Derived request;
    EXPECT_EQ(request.Login(root, rootPasswd),
              HttpRequest::LOGIN_STATUS::LOGIN_SUCCESS);
    EXPECT_EQ(request.Login(root, emptyPasswd),
              HttpRequest::LOGIN_STATUS::EMPTY_PASSWD);
    EXPECT_EQ(request.Login(emptyUser, rootPasswd),
              HttpRequest::LOGIN_STATUS::EMPTY_USERNAME);
    EXPECT_EQ(request.Login(root, wrongPasswd),
              HttpRequest::LOGIN_STATUS::WRONG_PASSWD);
    EXPECT_EQ(request.Login(wrongUser, rootPasswd),
              HttpRequest::LOGIN_STATUS::NO_SUCH_USER);
}