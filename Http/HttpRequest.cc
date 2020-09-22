#include "Http/HttpRequest.h"

using namespace std;

const unordered_set<string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::Init(){
    m_method = m_path = m_version = m_body = "";
    m_state = PARSE_STATE::REQUEST_LINE;
    m_header.clear();
    m_post.clear();
}

//TODO
bool HttpRequest::Parse(Buffer& buff){
    const char CRLF[] = "\r\n";
    if(buff.ReadableBytes() <= 0){
        return false;
    }
    while(buff.ReadableBytes() && m_state != PARSE_STATE::FINISH){
        const char* lineEnd = search(buff.Peek(), const_cast<const char*>(buff.BeginWrite()), CRLF, CRLF + 2);
        string line(buff.Peek(), lineEnd);
        LOG_INFO << line;
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
                //暂未测试，留到重构 DAO 后进行 mock 测试
                ParseBody(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);
    }
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
    LOG_ERROR << "Request Line Error : " << line;
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
    auto Content_Type = m_header.find("Content-Type");
    if(m_method == "POST" && Content_Type != m_header.end() && Content_Type->second == string("application/x-www-form-urlencoded")){
        ParseFromUrlencoded();
        auto Path_Tag = DEFAULT_HTML_TAG.find(m_path);
        if(Path_Tag != DEFAULT_HTML_TAG.end()){
            int tag = Path_Tag->second;
            LOG_DEBUG << "Tag[" << tag << "] ";
            if(tag == 1 || tag == 0){
                bool login = (tag == 1);
                if(UserVerify(m_post["username"], m_post["password"], login)){
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
        return Connection->second == "Keep-Alive";
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


bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool Login){
    if(name == "" || pwd == "") { return false; }
    LOG_INFO << "Verify name:" <<  name.c_str() << " , pwd:" << pwd.c_str();
    MYSQL* sql;
    connectionRAII sqlRAII(&sql,  ConnectionPool::GetInstance());
    assert(sql);
    
    bool flag = false;
    unsigned int j = 0;
    char order[256] = { 0 };
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;
    
    if(!Login) { flag = true; }
    /* 查询用户及密码 */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG << order;

    if(mysql_query(sql, order)) { 
        mysql_free_result(res);
        return false; 
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG << "MYSQL ROW: " << row[0] << " " << row[1];
        string password(row[1]);
        /* 注册行为 且 用户名未被使用*/
        if(Login) {
            if(pwd == password) { flag = true; }
            else {
                flag = false;
                LOG_DEBUG << "pwd error!";
            }
        } 
        else { 
            flag = false; 
            LOG_DEBUG << "user used!";
        }
    }
    mysql_free_result(res);

    /* 注册行为 且 用户名未被使用*/
    if(!Login && flag == true) {
        LOG_DEBUG << "regirster!";
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG <<  order;
        if(mysql_query(sql, order)) { 
            LOG_DEBUG << "Insert error!";
            flag = false; 
        }
        flag = true;
    }
    ConnectionPool::GetInstance()->ReleaseConnection(sql);
    LOG_DEBUG << "UserVerify success!!";
    return flag;
}