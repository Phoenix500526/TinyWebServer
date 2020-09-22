#include "Http/HttpResponse.h"

using namespace std;


HttpResponse::HttpResponse(){}
HttpResponse::~HttpResponse(){}

void HttpResponse::Init_Impl(const std::string& srcDir, std::string& path, bool isKeepAlive, int code){

}

void HttpResponse::UnmapFile(){}

void HttpResponse::MakeResponse(Buffer& buff){}

char* HttpResponse::File(){
    return nullptr;
}
size_t HttpResponse::FileLen() const{
    return 0;
}


void HttpResponse::ErrorContent(Buffer&, std::string){}

int HttpResponse::Code() const{
    return 0;
}
