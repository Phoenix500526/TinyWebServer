#include "Pools/SqlConnectionPool.h"
#include "Tools/CountDownLatch.h"
#include <unordered_map>
#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>

using namespace std;

ConnectionPool* connPool = ConnectionPool::GetInstance();
unordered_map<string, string> users;
vector<thread> thread_pool;



bool Register(string const& name, string const& passwd){
	if(users.find(name) != users.end()){
		cout << this_thread::get_id() << " : " << name << " exist!!" << endl;
		return false;
	}
	MYSQL* mysql = NULL;
	connectionRAII mysqlcon(&mysql, connPool);
	char sql_insert[200];
	snprintf(sql_insert, sizeof sql_insert, "INSERT INTO user_test(username, passwd) VALUES(\"%s\",\"%s\")", name.c_str(), passwd.c_str());
	if(mysql_query(mysql, sql_insert)){
		LOG_ERROR << "INSERT Failed : " << mysql_error(mysql);
	}
    users.insert(pair<string, string>(name, passwd));
	return true;
}

bool login(string const& name, string const& passwd){
	if(users.find(name) != users.end() && users[name] == passwd){
		cout << "login success!" << endl;
		return true;
	}
	cout << "no such user(" << name << ") or wrong password(" << passwd << ")" << endl;
	return false;
}

void showUsers(MYSQL* mysql){
	if (mysql_query(mysql, "SELECT username,passwd FROM user_test"))
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
	        users[temp1] = temp2;
	        cout  << "username = " << temp1 << " , and passwd = " << temp2 << endl;
	    }
}

void dropTheTable(){
	MYSQL* mysql = NULL;
	connectionRAII mysqlcon(&mysql, connPool);
	if(mysql_query(mysql, "DROP TABLE user_test;")){
		LOG_ERROR << "DROP TABLE user_test Failed : " << mysql_error(mysql);
	}
	cout << "drop success" << endl;
}

int main(void){
	connPool->init("localhost", "root", "123123", "zjwdb", 3306, 8);
	{
		MYSQL *mysql = NULL;
	    connectionRAII mysqlcon(&mysql, connPool);

	    //在user表中检索username，passwd数据，浏览器端输入
		if(mysql_query(mysql, "CREATE TABLE user_test(username VARCHAR(100) NOT NULL,passwd VARCHAR(60) NOT NULL,PRIMARY KEY ( username ));")){
			LOG_ERROR << "CREATE TABLE user_test Failed : " << mysql_error(mysql);
		}

		if(!Register("test1", "test1")){
			LOG_ERROR << "INSERT test user Failed";
		}
		if(!Register("test2", "test2")){
			LOG_ERROR << "INSERT test user Failed";
		}
		if(!Register("test3", "test3")){
			LOG_ERROR << "INSERT test user Failed";
		}

		showUsers(mysql);
	}
	CountDownLatch latch(4);
	thread_pool.emplace_back(login, "test1", "test1");
	//用户名不存在
	thread_pool.emplace_back(login, "test5", "test5");
	//密码错误的情况
	thread_pool.emplace_back(login, "test1", "test");
	//同时注册，并登录
	thread_pool.emplace_back([](CountDownLatch& latch, string const& name, string const& passwd)
		{
			latch.countDown();
			latch.wait();
			Register(name, passwd);
			login(name, passwd);
		}
	, std::ref(latch), "test_user1", "123123");
	thread_pool.emplace_back([](CountDownLatch& latch, string const& name, string const& passwd)
		{
			latch.countDown();
			latch.wait();
			Register(name, passwd);
			login(name, passwd);
		}
	, std::ref(latch), "test_user2", "123123");
	thread_pool.emplace_back([](CountDownLatch& latch, string const& name, string const& passwd)
		{
			latch.countDown();
			latch.wait();
			Register(name, passwd);
			login(name, passwd);
		}
	, std::ref(latch), "test_user3", "123123");
	thread_pool.emplace_back([](CountDownLatch& latch, string const& name, string const& passwd)
		{
			latch.countDown();
			latch.wait();
			Register(name, passwd);
			login(name, passwd);
		}
	, std::ref(latch), "test3", "test3");

	for(auto& th : thread_pool){
		if(th.joinable())
			th.join();
	}
	MYSQL* conn_temp = NULL;
	connectionRAII mysqlconn_temp(&conn_temp, connPool);
	showUsers(conn_temp);
	dropTheTable();
	return 0;
}