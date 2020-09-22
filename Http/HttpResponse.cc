#include "Http/HttpResponse.h"

using namespace std;

//TODO
HttpResponse::HttpResponse(){}

//TODO
HttpResponse::~HttpResponse(){}

//TODO
void HttpResponse::Init_Impl(const std::string& srcDir, std::string& path, bool isKeepAlive, int code){

}

//TODO
void HttpResponse::UnmapFile(){}

//TODO
void HttpResponse::MakeResponse(Buffer& buff){}

//TODO
char* HttpResponse::File(){
    return nullptr;
}

//TODO
size_t HttpResponse::FileLen() const{
    return 0;
}

//TODO
void HttpResponse::ErrorContent(Buffer&, std::string){}

//TODO
int HttpResponse::Code() const{
    return 0;
}

//TODO
void HttpResponse::AddStateLine_(Buffer &buff){

}

//TODO
void HttpResponse::AddHeader_(Buffer &buff){

}

//TODO
void HttpResponse::AddContent_(Buffer &buff){

}

//TODO
void HttpResponse::ErrorHtml_(){

}

//TODO
std::string HttpResponse::GetFileType(){
    return "";
}