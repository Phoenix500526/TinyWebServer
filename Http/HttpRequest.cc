#include "Http/HttpRequest.h"

using namespace std;

const unordered_set<string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

unordered_map<string, string> HttpRequest::USERTABLE;

void HttpRequest::Init(){
    m_method = m_path = m_version = m_body = "";
    m_state = PARSE_STATE::REQUEST_LINE;
    m_header.clear();
    m_post.clear();
}

bool HttpRequest::Parse(Buffer& buff){
    const char CRLF[] = "\r\n";
    if(buff.ReadableBytes() <= 0){
        return false;
    }
    while(buff.ReadableBytes() && m_state != PARSE_STATE::FINISH){
        const char* lineEnd = search(buff.Peek(), const_cast<const char*>(buff.BeginWrite()), CRLF, CRLF + 2);
        string line(buff.Peek(), lineEnd);
        switch(m_state){
            case PARSE_STATE::REQUEST_LINE:
                if(!ParseRequestLine(line))
                    return false;
                ParsePath();
                break;
            case PARSE_STATE::HEADERS:
                ParseHeader(line);
                if(buff.ReadableBytes() <= 2){
                    m_state = PARSE_STATE::FINISH;
                }
                break;
            case PARSE_STATE::BODY:
                ParseBody(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG << "[" << m_method <<  "], [" << m_path << "], [" << m_version << "]";
    return true;
}

bool HttpRequest::ParseRequestLine(const std::string& line){
    regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch matchRes;
    if(regex_match(line, matchRes, pattern)){
        m_method = matchRes[1];
        m_path = matchRes[2];
        m_version = matchRes[3];
        m_state = PARSE_STATE::HEADERS;
        return true;
    }
    return false;
}


void HttpRequest::ParseHeader(const std::string& line){
    regex pattern("^([^:]*): ?(.*)$");
    smatch matchRes;
    if(regex_match(line, matchRes, pattern)){
        m_header[matchRes[1]] = matchRes[2];
    }else{
        m_state = PARSE_STATE::BODY;
    }
}

void HttpRequest::ParseBody(const std::string& line){
    m_body = line;
    ParsePost();
    m_state = HttpRequest::PARSE_STATE::FINISH;
    LOG_DEBUG << "Body:" << m_body << ", len:" << m_body.size(); 
}

void HttpRequest::ParsePath(){
    if(m_path == "/"){
        m_path = "/index.html";
    }else{
        auto iter = DEFAULT_HTML.find(m_path);
        if(iter != DEFAULT_HTML.end()){
            m_path += ".html";
        }
    }
}

void HttpRequest::ParsePost(){
    if(m_method == "POST" && m_header["Content-Type"] == "application/x-www-form-urlencoded"){
        ParseFromUrlencoded();
        auto Path_Tag = DEFAULT_HTML_TAG.find(m_path);
        if(Path_Tag != DEFAULT_HTML_TAG.end()){
            int tag = Path_Tag->second;
            LOG_DEBUG << "Tag[" << tag << "] ";
            if(USERTABLE.empty()){
                if(!GetUserTableFromDB()){
                    LOG_ERROR << "Cannot Get USERTABLE from Database";
                }
            }
            if(tag == 1){
                if(Login(m_post["username"], m_post["password"]) == HttpRequest::LOGIN_STATUS::LOGIN_SUCCESS){
                    m_path = "/welcome.html";
                }else{
                    m_path = "/error.html";
                }
            }else{
                if(Register(m_post["username"], m_post["password"])){
                    m_path = "/welcome.html";
                }else{
                    m_path = "/error.html";
                }
            }
        }
    }
}

void HttpRequest::ParseFromUrlencoded(){
    if(m_body.empty())
        return;
    string key, value;
    int num = 0;
    int len = m_body.size();
    int i = 0, j = 0;
    while(i < len){
        char ch = m_body[i];
        switch(ch){
            case '=':
                key = m_body.substr(j, i - j);
                j = i + 1;
                break;
            case '&':
                value = m_body.substr(j, i - j);
                j = i + 1;
                m_post[key] = value;
                LOG_DEBUG << "Key and Value is : " <<  key << " : " << value;
                break;
            case '+':
                m_body[i] = ' ';
                break;
            case '%':
                num = ConverHex(m_body[i + 1]) * 16 + ConverHex(m_body[i + 2]);
                m_body[i + 2] = num % 10 + '0';
                m_body[i + 1] = num / 10 + '0';
                i += 2;
                break;
            default:
                break;
        }
        ++i;
    }
    assert(j <= i);
    if(m_post.find(key) == m_post.end() && j < i){
        value = m_body.substr(j, i - j);
        m_post[key] = value;
    }
}

int HttpRequest::ConverHex(char c){
    if(c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    assert(c >= '0' && c <= '9');
    return c;
}


bool HttpRequest::IsKeepAlive() const{
    auto Connection = m_header.find("Connection");
    if(m_version == "1.1" && Connection != m_header.end()){
        return Connection->second == "keep-alive";
    }
    return false;
}

string HttpRequest::path() const{
    return m_path;
}

string& HttpRequest::path(){
    return m_path;
}

string HttpRequest::method() const{
    return m_method;
}

string HttpRequest::version() const{
    return m_version;
}

string HttpRequest::HttpRequest::GetPost(const std::string& key) const{
    assert(!key.empty());
    return GetPost(key.c_str());
}

string HttpRequest::GetPost(const char* key) const{
    assert(key != nullptr);
    auto iter = m_post.find(key);
    if(iter != m_post.end()){
        return iter->second; 
    }
    return "";
}

HttpRequest::LOGIN_STATUS HttpRequest::Login(const std::string& name, const std::string& pwd){
    if(name.empty())
        return LOGIN_STATUS::EMPTY_USERNAME;
    if(pwd.empty())
        return LOGIN_STATUS::EMPTY_PASSWD;
    auto iter = USERTABLE.find(name);
    if(iter == USERTABLE.end()){
        return LOGIN_STATUS::NO_SUCH_USER;
    }
    if(iter->second != pwd)
        return LOGIN_STATUS::WRONG_PASSWD;
    return LOGIN_STATUS::LOGIN_SUCCESS;
}



bool HttpRequest::Register(const std::string& name, const std::string& pwd){
    if(name.empty() || pwd.empty())
        return false;
    if(USERTABLE.find(name) != USERTABLE.end())
        return false;
    MYSQL* sql;
    connectionRAII sqlRAII(&sql,  ConnectionPool::GetInstance());
    assert(sql);

    char order[256] = { 0 };
    snprintf(order, 256,"INSERT INTO user(username, passwd) VALUES('%s','%s')", name.c_str(), pwd.c_str());

    if(mysql_query(sql, order)) { 
        LOG_INFO << "Insert error!";
        return false; 
    }

    USERTABLE[name] = pwd;
    return true;
}


bool HttpRequest::GetUserTableFromDB(){
    MYSQL* sql;
    connectionRAII sqlRAII(&sql,  ConnectionPool::GetInstance());
    assert(sql);
    MYSQL_RES* res = nullptr;
    char order[256] = "SELECT username, passwd FROM user";
    LOG_DEBUG << order;

    if(mysql_query(sql, order)) { 
        mysql_free_result(res);
        return false; 
    }
    res = mysql_store_result(sql);
    while(MYSQL_ROW row = mysql_fetch_row(res)){
        if(USERTABLE.find(row[0]) == USERTABLE.end()){
            USERTABLE[row[0]] = row[1];
        }
    }    
    return true;
}