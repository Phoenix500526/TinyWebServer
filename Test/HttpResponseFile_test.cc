#include "Http/HttpResponse.h"
#include <unistd.h>
#include <iostream>
#include "Tools/Buffer.h"

using namespace std;

class HttpResponseFile_test : public HttpResponse {
 public:
  HttpResponseFile_test() {}
  ~HttpResponseFile_test() {}
  void AddContent(Buffer& buff) { HttpResponse::AddContent(buff); }
  void SetPath(string path) { HttpResponse::m_path = path; }
  void SetSrcDir(string srcDir) { HttpResponse::m_srcDir = srcDir; }
  string GetPath() { return HttpResponse::m_path; }
  string GetSrcDir() { return HttpResponse::m_srcDir; }
  char* GetFile() const { return HttpResponse::m_mmFile; }
  void MakeResponse(Buffer& buf) { return HttpResponse::MakeResponse(buf); }
};

void checkPointer(char* m_mmFile) {
  if (nullptr == m_mmFile) {
    cout << "m_mmFile is NULL\n";
  } else {
    cout << "m_mmFile is not NULL\n";
  }
}

int main(void) {
  {
    HttpResponseFile_test response_test;
    char* src = getcwd(nullptr, 256);
    response_test.SetSrcDir(src);
    response_test.SetPath("/HttpResponseFile_test.html");
    Buffer buf;
    cout << "Before AddContent:\n";
    checkPointer(response_test.GetFile());
    // response_test.AddContent(buf);
    response_test.MakeResponse(buf);
    cout << buf.RetrieveAllAsString() << endl;
  }
  return 0;
}