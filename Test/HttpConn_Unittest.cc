#include "Http/HttpConn.h"
#include "Http/HttpRequest.h"
#include "Http/HttpResponse.h"
#include "Tools/Buffer.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::Return;
using ::testing::_;  // Matcher for parameters
using ::testing::ReturnRef;
using ::testing::AtLeast;

bool operator==(const sockaddr_in& lhs, const sockaddr_in& rhs) {
  return lhs.sin_port == rhs.sin_port &&
         !strcmp(inet_ntoa(lhs.sin_addr), inet_ntoa(rhs.sin_addr));
}

bool operator==(const Buffer& lhs, const Buffer& rhs) {
  return lhs.m_buffer == rhs.m_buffer &&
         lhs.m_writerIndex == rhs.m_writerIndex &&
         lhs.m_readerIndex == rhs.m_readerIndex;
}

class MockHttpRequest : public HttpRequestBase {
 public:
  MockHttpRequest() {}

  MOCK_METHOD0(Init, void());
  MOCK_METHOD1(Parse, bool(Buffer&));
  MOCK_CONST_METHOD0(IsKeepAlive, bool());
  MOCK_CONST_METHOD0(path, std::string());
  MOCK_METHOD0(path, std::string&());
  MOCK_CONST_METHOD0(method, std::string());
  MOCK_CONST_METHOD0(version, std::string());
  MOCK_CONST_METHOD1(GetPost, std::string(const std::string&));
  MOCK_CONST_METHOD1(GetPost, std::string(const char*));
};

class MockHttpResponse : public HttpResponseBase {
 public:
  MockHttpResponse() {}
  void Init(const std::string& srcDir, std::string& path,
            bool IsKeepAlive = false, int code = -1) {
    Init_Impl(srcDir, path, IsKeepAlive, code);
  }
  MOCK_METHOD4(Init_Impl, void(const std::string&, std::string&, bool, int));
  MOCK_METHOD0(UnmapFile, void());
  MOCK_METHOD1(MakeResponse, void(Buffer& buff));
  MOCK_METHOD0(File, char*());
  MOCK_CONST_METHOD0(FileLen, size_t());
  MOCK_METHOD2(ErrorContent, void(Buffer&, std::string));
  MOCK_CONST_METHOD0(Code, int());
};

class HttpConnDerived : public HttpConn {
 public:
  HttpConnDerived() = default;
  ~HttpConnDerived() = default;
  void AppendRead(const std::string& str) { HttpConn::m_readBuff.Append(str); }

  void AppendWrite(const std::string& str) {
    HttpConn::m_writeBuff.Append(str);
  }

  Buffer& GetReadBuff() { return HttpConn::m_readBuff; }

  Buffer& GetWriteBuff() { return HttpConn::m_writeBuff; }

  void SetResponse(std::shared_ptr<MockHttpResponse> response_) {
    HttpConn::m_response = response_;
  }

  void SetRequest(std::shared_ptr<MockHttpRequest> request_) {
    HttpConn::m_request = request_;
  }
};

class HttpConnTest : public ::testing::Test {
 protected:
  HttpConnTest() {
    HttpConn::srcDir = "resources/";
    HttpConn::isET = false;
    HttpConn::userCnt = 0;
  }
  ~HttpConnTest() {}
};

TEST_F(HttpConnTest, HttpInit) {
  std::shared_ptr<MockHttpResponse> response_ =
      std::make_shared<MockHttpResponse>();
  {
    std::shared_ptr<MockHttpRequest> request_ =
        std::make_shared<MockHttpRequest>();
    HttpConn http;

    http.SetRequest(request_);
    http.SetResponse(response_);

    ASSERT_EQ(http.GetFd(), -1) << "http's fd is " << http.GetFd();
    struct sockaddr_in res {};
    ASSERT_EQ(http.GetAddr(), res) << "http's Addr is " << http.GetIP() << " : "
                                   << http.GetPort();

    struct sockaddr_in addr = {1, 1};

    http.Init(10, addr);
    EXPECT_EQ(http.GetFd(), 10);
    EXPECT_EQ(http.GetAddr(), addr);
    EXPECT_EQ(HttpConn::userCnt.load(), 1);

    EXPECT_CALL(*response_, UnmapFile()).Times(1);
  }
  EXPECT_EQ(HttpConn::userCnt.load(), 0);
}

TEST_F(HttpConnTest, HttpProcessSuccessTest) {
  std::shared_ptr<MockHttpResponse> response_ =
      std::make_shared<MockHttpResponse>();
  std::shared_ptr<MockHttpRequest> request_ =
      std::make_shared<MockHttpRequest>();
  HttpConnDerived http;
  http.SetResponse(response_);
  http.SetRequest(request_);
  http.AppendRead("Hello World");
  http.AppendWrite("Hello World");

  Buffer& m_readBuff = http.GetReadBuff();
  Buffer& m_writeBuff = http.GetWriteBuff();

  std::string path_res("resources");

  EXPECT_CALL(*request_, Init()).Times(1);

  EXPECT_CALL(*request_, Parse(m_readBuff)).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*request_, IsKeepAlive()).Times(1).WillOnce(Return(true));

  EXPECT_CALL(*response_, Init_Impl(HttpConn::srcDir, path_res, true, 200))
      .Times(1);

  EXPECT_CALL(*request_, path())
      .Times(AtLeast(1))
      .WillOnce(ReturnRef(path_res));

  EXPECT_CALL(*response_, MakeResponse(m_writeBuff)).Times(1);

  EXPECT_CALL(*response_, FileLen()).Times(1).WillOnce(Return(0));

  http.Process();
  EXPECT_CALL(*response_, UnmapFile()).Times(1);
}

TEST_F(HttpConnTest, HttpProcessFailedTest) {
  std::shared_ptr<MockHttpResponse> response_ =
      std::make_shared<MockHttpResponse>();
  std::shared_ptr<MockHttpRequest> request_ =
      std::make_shared<MockHttpRequest>();
  HttpConn http;
  http.SetResponse(response_);
  http.SetRequest(request_);

  EXPECT_EQ(http.Process(), false);
}