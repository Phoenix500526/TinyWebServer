#include "Http/HttpResponse.h"

using namespace std;

const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css"},
    { ".js",    "text/javascript"},
};

const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const unordered_map<int, string> HttpResponse::ERROR_PAGE = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HttpResponse::HttpResponse()
    :m_code(-1),
     m_isKeepAlive(false),
     m_path(""), 
     m_srcDir(""), 
     m_mmFile(nullptr),
     m_mmFileStat({0}){}


HttpResponse::~HttpResponse(){
    UnmapFile();
}


void HttpResponse::Init_Impl(const std::string& srcDir, std::string& path, bool isKeepAlive, int code){
    //assert(srcDir.empty() && path.empty());
    m_srcDir = srcDir;
    m_path = path;
    m_isKeepAlive = isKeepAlive;
    m_code = code;
    m_mmFile = nullptr;
    m_mmFileStat = {0};
}


void HttpResponse::UnmapFile(){
    if(m_mmFile){
        munmap(m_mmFile, m_mmFileStat.st_size);
        m_mmFile = nullptr;
    }
}

void HttpResponse::MakeResponse(Buffer& buff){
    /* 判断请求的资源文件 */
    if(stat((m_srcDir + m_path).data(), &m_mmFileStat) < 0 || S_ISDIR(m_mmFileStat.st_mode)) {
        m_code = 404;
    }
    else if(!(m_mmFileStat.st_mode & S_IROTH)) {
        m_code = 403;
    }
    else if(m_code == -1) { 
        m_code = 200; 
    }
    ErrorHtml();
    AddStateLine(buff);
    AddHeader(buff);
    AddContent(buff);
}


char* HttpResponse::File(){
    return m_mmFile;
}


size_t HttpResponse::FileLen() const{
    return m_mmFileStat.st_size;
}


int HttpResponse::Code() const{
    return m_code;
}

const string HttpResponse::checkStatus(){
    string status;
    auto iter = CODE_STATUS.find(m_code);
    if(iter != CODE_STATUS.end()){
        status = iter->second;
    }else{
        m_code = 400;
        status = CODE_STATUS.find(m_code)->second;
    }
    return status;
}

void HttpResponse::AddStateLine(Buffer &buff){
    string status = checkStatus();
    buff.Append("HTTP/1.1 " + to_string(m_code) + " " + status + "\r\n");
}


void HttpResponse::AddHeader(Buffer &buff){
    buff.Append("Connection: ");
    if(m_isKeepAlive){
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    }else{
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType() + "\r\n");
}


void HttpResponse::AddContent(Buffer &buff){
    int srcFd = open((m_srcDir + m_path).c_str(), O_RDONLY);
    if(srcFd < 0){
        ErrorContent(buff, "File Not Found");
        return;
    }
    LOG_DEBUG << "path is " << (m_srcDir + m_path).c_str();

    //mapping file into memory for speeding up the file accessing process.
    void* mmRet = mmap(nullptr, m_mmFileStat.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(mmRet == MAP_FAILED){
        ErrorContent(buff, "File Not Found");
        return;
    }
    m_mmFile = static_cast<char*>(mmRet);
    close(srcFd);
    buff.Append("Content-length: " + to_string(m_mmFileStat.st_size) + "\r\n\r\n");
}

void HttpResponse::ErrorHtml(){
    auto iter = ERROR_PAGE.find(m_code);
    if(iter != ERROR_PAGE.end()){
        m_path = iter->second;
        stat((m_srcDir + m_path).c_str(), &m_mmFileStat);
    }
}

void HttpResponse::ErrorContent(Buffer& buff, std::string message){
    string body;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    //FIXME:Is it a must to change the status code?
    body += to_string(m_code);
    body += " : " + checkStatus()  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-Length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}


std::string HttpResponse::GetFileType(){
    string::size_type idx = m_path.find_last_of('.');
    if(idx == string::npos){
        return SUFFIX_TYPE.find(".txt")->second;
    }
    string suffix = m_path.substr(idx);
    auto iter = SUFFIX_TYPE.find(suffix);
    if(iter != SUFFIX_TYPE.end()){
        return iter->second;
    }
    return SUFFIX_TYPE.find(".txt")->second;
}