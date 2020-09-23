#include <gtest/gtest.h>
#include "Tools/Buffer.h"
#include "Http/HttpResponse.h"


class HttpResponse_Derive:public HttpResponse{
public:
    HttpResponse_Derive():HttpResponse(){}
    ~HttpResponse_Derive(){}
    char* GetFile(){
        return HttpResponse::File();
    }
    size_t GetFileLen() const{
        return HttpResponse::FileLen();
    }
    
    int GetCode() const{
        return HttpResponse::Code();
    }

    std::string GetSrcDir()const{
        return HttpResponse::m_srcDir;
    }

    std::string GetPath()const{
        return HttpResponse::m_path;
    }

    bool GetIsKeepAlive()const{
        return HttpResponse::m_isKeepAlive;
    }

    void SetCode(int code){
        HttpResponse::m_code = code;
    }
    void SetFile(char* file){
        HttpResponse::m_mmFile = file;
    }
    void SetSrcDir(const std::string& new_srcDir){
        HttpResponse::m_srcDir = new_srcDir;
    }
    void SetPath(const std::string& new_path){
        HttpResponse::m_path = new_path;
    }
    void SetIsKeepAlive(bool iskeepAlive){
        HttpResponse::m_isKeepAlive = iskeepAlive;
    }
    std::string GetFileType(){
        return HttpResponse::GetFileType();
    }
    void ErrorHtml(){
        HttpResponse::ErrorHtml();
    }
    void AddStateLine(Buffer &buff){
        HttpResponse::AddStateLine(buff);
    }
    void AddHeader(Buffer &buff){
        HttpResponse::AddHeader(buff);
    }
};

TEST(HttpResponseUnittest, InitTest){
    HttpResponse_Derive response;
    EXPECT_EQ(response.GetCode(), -1);
    EXPECT_EQ(response.GetFile(), nullptr);
    EXPECT_EQ(response.GetPath(), std::string(""));
    EXPECT_EQ(response.GetSrcDir(), std::string(""));
    EXPECT_EQ(response.GetIsKeepAlive(), false);

    std::string srcDir("Hello");
    std::string path("World");
    response.Init(srcDir, path, true, 10);
    EXPECT_EQ(response.GetCode(), 10);
    EXPECT_EQ(response.GetFile(), nullptr);
    EXPECT_EQ(response.GetPath(), std::string("World"));
    EXPECT_EQ(response.GetSrcDir(), std::string("Hello"));
    EXPECT_EQ(response.GetIsKeepAlive(), true);

}

TEST(HttpResponseUnittest, GetFileTypeTest){
    HttpResponse_Derive response;
    response.SetPath("/helloworld.html");
    EXPECT_EQ(response.GetFileType(), std::string("text/html"));
    response.SetPath("/helloworld.jpg");
    EXPECT_EQ(response.GetFileType(), std::string("image/jpeg"));
    response.SetPath("/helloworld.js");
    EXPECT_EQ(response.GetFileType(), std::string("text/javascript"));
    response.SetPath("helloworld");
    EXPECT_EQ(response.GetFileType(), std::string("text/plain"));
}

TEST(HttpResponseUnittest, ErrorTest){
    HttpResponse_Derive response;
    response.SetCode(200);
    response.ErrorHtml();
    response.SetPath("helloworld");
    EXPECT_EQ(response.GetPath(), "helloworld");
    response.SetCode(400);
    response.ErrorHtml();
    EXPECT_EQ(response.GetPath(), "/400.html");
    response.SetCode(403);
    response.ErrorHtml();
    EXPECT_EQ(response.GetPath(), "/403.html");
    response.SetCode(404);
    response.ErrorHtml();
    EXPECT_EQ(response.GetPath(), "/404.html");
}

TEST(HttpResponseUnittest, AddStateLineTest){
    HttpResponse_Derive response;
    response.SetCode(200);

    Buffer stateLineBuf;
    response.AddStateLine(stateLineBuf);
    std::string res_OK = stateLineBuf.RetrieveAllAsString();
    EXPECT_EQ(res_OK, std::string("HTTP/1.1 200 OK\r\n"));

    response.SetCode(400);
    response.AddStateLine(stateLineBuf);
    std::string res_BadRequest = stateLineBuf.RetrieveAllAsString();
    EXPECT_EQ(res_BadRequest, std::string("HTTP/1.1 400 Bad Request\r\n"));

    response.SetCode(403);
    response.AddStateLine(stateLineBuf);
    std::string res_Forbidden = stateLineBuf.RetrieveAllAsString();
    EXPECT_EQ(res_Forbidden, std::string("HTTP/1.1 403 Forbidden\r\n"));
}

TEST(HttpResponseUnittest, AddHeaderTest){
    HttpResponse_Derive response;
    response.SetIsKeepAlive(true);
    response.SetPath("helloworld.html");
    Buffer headerBuf;

    response.AddHeader(headerBuf);
    std::string keepAliveHeader = "Connection: Keep-Alive\r\nKeep-Alive: timeout=10000\r\nContent-Type: text/html\r\n";
    EXPECT_EQ(headerBuf.RetrieveAllAsString(), keepAliveHeader);

    response.SetIsKeepAlive(false);
    response.AddHeader(headerBuf);
    std::string closeHeader = "Connection: Close\r\nContent-Type: text/html\r\n";
    EXPECT_EQ(headerBuf.RetrieveAllAsString(), closeHeader);
}

TEST(HttpResponseUnittest, ErrorContentTest){
    HttpResponse_Derive response;
    response.SetCode(500);

    Buffer errorBuf;
    response.ErrorContent(errorBuf, "File Not Found");

    std::string ErrorContent_BadRequest("<html><title>Error</title><body bgcolor=\"ffffff\">500 : Bad Request\n<p>File Not Found</p><hr><em>TinyWebServer</em></body></html>");
    std::string result_1 = "Content-Length: " + std::to_string(ErrorContent_BadRequest.size()) + "\r\n\r\n" + ErrorContent_BadRequest;
    EXPECT_EQ(errorBuf.RetrieveAllAsString(), result_1);
    
    response.SetCode(403);
    response.ErrorContent(errorBuf, "Permission Denied");
    std::string ErrorContent_PermissionDenied("<html><title>Error</title><body bgcolor=\"ffffff\">403 : Forbidden\n<p>Permission Denied</p><hr><em>TinyWebServer</em></body></html>");
    std::string result_2 = "Content-Length: " + std::to_string(ErrorContent_PermissionDenied.size()) + "\r\n\r\n" + ErrorContent_PermissionDenied;
    EXPECT_EQ(errorBuf.RetrieveAllAsString(), result_2);
}