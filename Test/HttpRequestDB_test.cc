#include "Http/HttpConn.h"
#include "Http/HttpRequest.h"
#include "Http/HttpResponse.h"
#include "Pools/SqlConnectionPool.h"

#include <iostream>

using namespace std;

ConnectionPool* connPool = ConnectionPool::GetInstance();
unordered_map<string, string> users;


class HttpRequest_test:public HttpRequest
{
public:
    HttpRequest_test():HttpRequest(){}
    ~HttpRequest_test(){}
    static bool GetUserTableFromDB(){
        return HttpRequest::GetUserTableFromDB();
    }
    static void show(){
        cout << "######################################################\n";
        cout << "USERTABLE:\n";
        for (auto const& iter:HttpRequest::USERTABLE)
        {
            cout << "username = " << iter.first << ", passwd = " << iter.second << endl;
        }
        cout << "######################################################" << endl;
    }
    bool Register(const string& name, const string& pwd){
        return HttpRequest::Register(name, pwd);
    }
    HttpRequest::LOGIN_STATUS Login(const string& name, const string& pwd){
        return HttpRequest::Login(name, pwd);
    }
};

void showUsers(MYSQL* mysql){
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
        {
            LOG_ERROR << "SELECT error:%s\n", mysql_error(mysql);
        }

        //从表中检索完整的结果集
        MYSQL_RES *result = mysql_store_result(mysql);

        //从结果集中获取下一行，将对应的用户名和密码，存入map中
        while (MYSQL_ROW row = mysql_fetch_row(result))
        {
            string temp1(row[0]);
            string temp2(row[1]);
            cout  << "username = " << temp1 << " , and passwd = " << temp2 << endl;
        }
}



int main(void){
    connPool->init("localhost", "root", "123123", "zjwdb", 3306, 8);
    
    MYSQL *mysql = nullptr;
    connectionRAII mysqlcon(&mysql, connPool);

    char order[] = "INSERT INTO user(username, passwd) VALUES(\"http_login_test\",\"123123\")";
    cout << "Inserting test user ...\n";
    if(mysql_query(mysql, order)) { 
        cout << "Insert error!";
        return -1; 
    }
    cout << "Insert test user success : username = http_login_test, passwd = 123123\n"; 

    if(HttpRequest_test::GetUserTableFromDB()){
        HttpRequest_test::show();
    }else{
        cout << "error" << endl;
    }

    HttpRequest_test test;
    string user("http_login_test");
    string passwd("123123");
    if(HttpRequest::LOGIN_STATUS::LOGIN_SUCCESS == test.Login(user, passwd)){
        cout << "Login success: username = " << user << ", passwd = " << passwd << '\n';
    }

    if(!test.Register(user, passwd)){
        cout << "Username has been Registered : " << user << endl;
    }else{
        cout << "Register Success: " << user << '\n';
    }

    string new_user("http_register_test");
    if(!test.Register(new_user, passwd)){
        cout << "Username has been Registered: " << new_user << endl;
    }else{
        cout << "Register Success: " << new_user << '\n';
    }
    cout << "before delete:\n";

    showUsers(mysql);
    cout << "deleting user http_login_test ...\n";
    char del_order1[] = "DELETE FROM user WHERE username = \"http_login_test\"";
    if(mysql_query(mysql, del_order1)) { 
        cout << "delete http_login_test fauilare!"; 
    }
    cout << "delete user http_login_test success\n"; 

    char del_order2[] = "DELETE FROM user WHERE username = \"http_register_test\"";
    if(mysql_query(mysql, del_order2)) { 
        cout << "delete http_register_test fauilare!"; 
    }
    cout << "delete user http_register_test success\n"; 
    cout << "After delete:\n"; 
    showUsers(mysql);
    return 0;
}