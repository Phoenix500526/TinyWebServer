#ifndef TINYWEBSERVER_POOL_DAO
#define TINYWEBSERVER_POOL_DAO

#include <mysql/mysql.h>
#include <unordered_map>

class DAO{
public:
    virtual bool Insert(std::string const&, std::string const&) = 0;
    virtual bool Select(std::unordered_map<std::string, std::string>&) const = 0;
    virtual ~DAO(){}
};

class MYSQLDAO:public DAO
{
public:
    MYSQLDAO(std::string const& url, std::string const& User,
        std::string const& PassWord, std::string const& DBName,
        int SqlPort):con(nullptr){
        con = mysql_init(con);

        if (con == nullptr)
        {
            LOG_FATAL << "MySQL Init Error";
        }
        con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), SqlPort, nullptr, 0);

        if (con == nullptr)
        {
            LOG_FATAL << "MySQL Get Real Connection Error";
        }
    }
    ~MYSQLDAO(){
        mysql_close(con);
    }
    bool Insert(std::string const& name, std::string const& pwd) override{
        char order[256] = { 0 };
        snprintf(order, 256,"INSERT INTO user(username, passwd) VALUES('%s','%s')", name.c_str(), pwd.c_str());

        if(mysql_query(con, order)) { 
            LOG_ERROR << "Insert error!";
            return false; 
        }
        return true;
    }

    bool Select(std::unordered_map<std::string, std::string>& table) const override{
        MYSQL_RES* res = nullptr;
        char order[256] = "SELECT username, passwd FROM user";
        if(mysql_query(con, order)) { 
            mysql_free_result(res);
            return false; 
        }
        res = mysql_store_result(con);
        while(MYSQL_ROW row = mysql_fetch_row(res)){
            table[row[0]] = row[1];
        }    
        return true;
    }

protected:
    MYSQL* con;
};
#endif